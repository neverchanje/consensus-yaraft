import subprocess
import time
import unittest
import os, shutil
import requests
import json


class MemKvNode:
    def __init__(self, id):
        self.id = id

    def node_dir(self):
        return os.getcwd() + "/server{}.consensus".format(self.id)

    def wal_dir(self):
        return self.node_dir() + "/wal"

    def log_dir(self):
        return self.node_dir() + "/log"

    def address(self):
        return "127.0.0.1:{}".format(12320 + self.id)


class RaftProcess:
    def __init__(self, cmdline, process, output):
        self.cmdline = cmdline
        self.process = process
        self.output = output
        print cmdline


class LocalCluster:
    def __init__(self, num):
        self.p = []
        self.server_count = num
        self.nodes = [MemKvNode(i + 1) for i in range(0, self.server_count)]
        self.clients = [RaftClient(self.nodes[i].address()) for i in range(0, self.server_count)]

    # set up `server_count` number of servers.
    def __enter__(self):
        nodes = self.nodes

        for i in range(0, self.server_count):
            # remove node dir if it exists.
            if os.path.exists(nodes[i].node_dir()):
                shutil.rmtree(nodes[i].node_dir())
            os.mkdir(nodes[i].node_dir())
            os.mkdir(nodes[i].wal_dir())
            os.mkdir(nodes[i].log_dir())

            wal_dir = "--wal_dir={}".format(nodes[i].wal_dir())
            log_dir = "--memkv_log_dir={}".format(nodes[i].log_dir())
            id = "--id={}".format(i + 1)
            server_num = "--server_count={}".format(self.server_count)

            out = open(nodes[i].node_dir() + "/error.log", "w")
            cmdline = ["../output/bin/memkv_server", id, wal_dir, server_num, log_dir]
            process = subprocess.Popen(cmdline, stderr=out)
            self.p.append(RaftProcess(cmdline, process, out))

        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        for i in range(0, self.server_count):
            self.p[i].process.kill()
            self.p[i].output.close()

    def node(self, id):
        return self.nodes[id - 1]

    def restart(self, id):
        p = self.p[id - 1]
        p.process.kill()
        p.process = subprocess.Popen(p.cmdline, stderr=p.output)

    def kill(self, id):
        p = self.p[id - 1]
        p.process.kill()


class RaftClient:
    def __init__(self, url):
        self.url = url

    def write(self, str):
        pass

    def _do_read(self):
        pass

    def _do_info(self):
        resp = requests.get("http://{}/RaftService/Status".format(self.url))
        return json.loads(resp.content)

    def info(self):
        return request_with_retry(self._do_info, 5)


# retry for 5 times until the remote node becomes valid or dead.
def request_with_retry(do_func, retries_num):
    retry = 0
    while True:
        try:
            ret = do_func()
            if ret is not None:
                return ret
            break  # break if everything goes ok
        except requests.ConnectionError as e:
            retry += 1
            if retry > retries_num:
                raise e
            time.sleep(1)


class RaftServerTest(unittest.TestCase):
    def test_LeaderElectionBootstrap(self):
        for server_count in range(1, 8):
            with LocalCluster(server_count) as cluster:
                while RaftClient(cluster.node(1).address()).info()["leader"] == 0:
                    time.sleep(1)

                leader = RaftClient(cluster.node(1).address()).info()["leader"]
                for i in range(0, server_count):
                    info = RaftClient(cluster.node(i + 1).address()).info()

                    self.assertEqual(info["raftTerm"], 1)
                    self.assertEqual(info["leader"], leader)

    # this test verifies that the cluster will reelect a new leader when the old one falls down.
    def test_LeaderDownReelection(self):
        for server_count in range(3, 6):
            with LocalCluster(server_count) as cluster:
                while RaftClient(cluster.node(1).address()).info()["leader"] == 0:
                    time.sleep(1)

                old_leader = RaftClient(cluster.node(1).address()).info()["leader"]
                info = RaftClient(cluster.node(old_leader).address()).info()
                self.assertEqual(info["raftTerm"], 1)

                cluster.kill(old_leader)

                # find a node that is not current leader
                non_leader = 0
                for i in range(0, server_count):
                    if (i + 1) != old_leader:
                        non_leader = i + 1
                        break

                # wait until the leadership changes
                while RaftClient(cluster.node(non_leader).address()).info()["leader"] == old_leader:
                    time.sleep(1)

                new_leader = RaftClient(cluster.node(non_leader).address()).info()["leader"]
                for i in range(0, server_count):
                    if (i + 1) == old_leader:
                        continue
                    info = RaftClient(cluster.node(i + 1).address()).info()

                    self.assertEqual(info["raftTerm"], 2)
                    self.assertEqual(info["leader"], new_leader)


if __name__ == '__main__':
    unittest.main()
