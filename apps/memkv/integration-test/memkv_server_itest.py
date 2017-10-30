import subprocess
import time
import unittest
import os, shutil
import requests
import json
import string, random


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
        time.sleep(1)
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

    # create client for id
    def client(self, id):
        return RaftClient(self.node(id).address())


class RaftClient:
    def __init__(self, url):
        self.url = url

    def write(self, path, value):
        return request_with_retry(self._do_write, 2, path, value)

    def _do_write(self, path, value):
        resp = requests.get("http://{}/MemKVService/Write/{}?value={}".format(self.url, path, value))
        return json.loads(resp.content)

    def read(self, path):
        return request_with_retry(self._do_read, 2, path)

    def _do_read(self, path):
        resp = requests.get("http://{}/MemKVService/Read/{}".format(self.url, path))
        return json.loads(resp.content)

    def _do_info(self):
        resp = requests.get("http://{}/RaftService/Status".format(self.url))
        return json.loads(resp.content)

    def info(self):
        return request_with_retry(self._do_info, 5)

    def wait_until_leader_elected(self, old_leader=0):
        while self.info()["leader"] == old_leader:
            time.sleep(1)
        return self.info()["leader"]


# retry for 5 times until the remote node becomes valid or dead.
def request_with_retry(do_func, retries_num, *args):
    retry = 0
    while True:
        try:
            ret = None
            if len(args) == 0:
                ret = do_func()
            else:
                ret = do_func(*args)
            if ret is not None:
                return ret
            break  # break if everything goes ok
        except requests.ConnectionError as e:
            retry += 1
            if retry > retries_num:
                raise e
            time.sleep(1)


def random_string(length):
    return ''.join(random.choice(string.lowercase) for x in range(length))


class RaftServerTest(unittest.TestCase):
    def test_LeaderElectionBootstrap(self):
        for server_count in range(1, 8):
            with LocalCluster(server_count) as cluster:
                leader = cluster.client(1).wait_until_leader_elected()

                for i in range(0, server_count):
                    info = cluster.client(i + 1).info()

                    self.assertEqual(info["raftTerm"], 1)
                    self.assertEqual(info["leader"], leader)

    # this test verifies that the cluster will reelect a new leader when the old one falls down.
    def test_LeaderDownReelection(self):
        for server_count in range(3, 6):
            with LocalCluster(server_count) as cluster:
                old_leader = cluster.client(1).wait_until_leader_elected()
                info = cluster.client(old_leader).info()
                self.assertEqual(info["raftTerm"], 1)

                cluster.kill(old_leader)

                # find a node that is not current leader
                non_leader = 0
                for i in range(0, server_count):
                    if (i + 1) != old_leader:
                        non_leader = i + 1
                        break

                # wait until the leadership changes
                new_leader = cluster.client(non_leader).wait_until_leader_elected(old_leader)

                for i in range(0, server_count):
                    if (i + 1) == old_leader:
                        continue
                    info = cluster.client(i + 1).info()

                    self.assertEqual(info["raftTerm"], 2)
                    self.assertEqual(info["leader"], new_leader)

    # this test verifies that using the same keys can read the same value.
    def test_WriteAndRead(self):
        for server_count in [1, 3, 5, 7]:
            mem_table = {}
            for i in range(5):
                mem_table[random_string(10)] = random_string(20)

            with LocalCluster(server_count) as cluster:
                leader = cluster.client(1).wait_until_leader_elected()

                for key, value in mem_table.iteritems():
                    cluster.client(leader).write(key, value)

                for key, value in mem_table.iteritems():
                    self.assertEqual(cluster.client(leader).read(key)['value'], value)


if __name__ == '__main__':
    unittest.main()
