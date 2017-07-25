import subprocess
import time
import unittest
import os, shutil
import requests
import json


class RaftNode:
    def __init__(self, id):
        self.id = id

    def wal_dir(self):
        return "infra{}.consensus".format(self.id)

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
        self.server_num = num
        self.nodes = [RaftNode(i + 1) for i in range(0, self.server_num)]
        self.clients = [RaftClient(self.nodes[i].address()) for i in range(0, self.server_num)]

    # set up `server_num` number of servers.
    def __enter__(self):
        nodes = self.nodes

        for i in range(0, self.server_num):
            if os.path.isdir(nodes[i].wal_dir()):
                shutil.rmtree(nodes[i].wal_dir())

            wal_dir = "--wal_dir={}".format(nodes[i].wal_dir())
            id = "--id={}".format(i + 1)
            server_num = "--server_num={}".format(self.server_num)

            out = open("server{}.log".format(i + 1), "w")

            cmdline = ["../output/bin/raft_server", id, wal_dir, server_num]
            process = subprocess.Popen(cmdline, stderr=out)
            self.p.append(RaftProcess(cmdline, process, out))

        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        for i in range(0, self.server_num):
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

    def info(self, id):
        self.clients[id - 1].Info()


class RaftClient:
    def __init__(self, url):
        self.url = url

    def Write(self, str):
        pass

    def Info(self):
        return json.loads(requests.get("http://" + self.url + "/info").content)


class RaftServerTest(unittest.TestCase):
    def test_LeaderElectionBootstrap(self):
        for server_num in range(1, 6):
            with LocalCluster(server_num) as cluster:
                time.sleep(server_num)

                while RaftClient(cluster.node(1).address()).Info()["currentLeader"] == 0:
                    time.sleep(1)

                info = RaftClient(cluster.node(1).address()).Info()
                self.assertEqual(info["currentTerm"], 1)
                self.assertEqual(info["logIndex"], 1)

    # this test verifies that the cluster will reelect a new leader when the old one falls down.
    def test_LeaderDownReelection(self):
        for server_num in range(3, 4):
            with LocalCluster(server_num) as cluster:
                time.sleep(server_num)

                while RaftClient(cluster.node(1).address()).Info()["currentLeader"] == 0:
                    time.sleep(1)

                leader = RaftClient(cluster.node(1).address()).Info()["currentLeader"]
                info = RaftClient(cluster.node(leader).address()).Info()
                self.assertEqual(info["currentTerm"], 1)
                self.assertEqual(info["logIndex"], 1)

                cluster.kill(leader)

                nonLeader = 0
                for i in range(0, server_num):
                    if (i + 1) != leader:
                        nonLeader = i + 1
                        break

                while RaftClient(cluster.node(nonLeader).address()).Info()["currentLeader"] == leader:
                    time.sleep(1)

                leader = RaftClient(cluster.node(nonLeader).address()).Info()["currentLeader"]
                info = RaftClient(cluster.node(leader).address()).Info()
                self.assertEqual(info["currentTerm"], 2)
                self.assertEqual(info["logIndex"], 2)


if __name__ == '__main__':
    unittest.main()
