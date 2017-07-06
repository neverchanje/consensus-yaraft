import subprocess
import sys
import time
from sofa.pbrpc import client
import raft_server_pb2, yaraft.pb.raftpb_pb2
import unittest
import os, shutil


class RaftNode:
    def __init__(self, id):
        self.id = id

    def name(self):
        return "infra{}".format(self.id)

    def address(self):
        return "127.0.0.1:{}".format(self.id + 12320)

    def wal_dir(self):
        return "{}.consensus".format(self.name())


class RaftProcess:
    def __init__(self, cmdline, process, output):
        self.cmdline = cmdline
        self.process = process
        self.output = output


class LocalCluster:
    def __init__(self, num):
        self.p = []
        self.server_num = num
        self.nodes = [RaftNode(i + 1) for i in range(0, self.server_num)]

    # set up `server_num` number of servers.
    def __enter__(self):
        nodes = self.nodes
        addresses = ["{}={}".format(nodes[i].name(), nodes[i].address()) for i in range(0, self.server_num)]
        initial_cluster = "--initial_cluster=" + ';'.join(addresses)

        for i in range(0, self.server_num):
            if os.path.isdir(nodes[i].wal_dir()):
                shutil.rmtree(nodes[i].wal_dir())

            wal_dir = "--wal_dir={}".format(nodes[i].wal_dir())
            name = "--name={}".format(nodes[i].name())
            out = open("server{}.log".format(i + 1), "w")

            cmdline = ["../output/bin/raft_server", initial_cluster, wal_dir, name]
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


class RaftClient:
    def __init__(self, url):
        self.channel = client.Channel(url)

    def doRequest(self, requestFunc, *args):
        service = raft_server_pb2.RaftService_Stub(self.channel)
        controller = client.Controller()
        controller.SetTimeout(1.5)

        try:
            response = requestFunc(service, controller, args)
        except client.TimeoutError:
            print "ERROR: RPC timeout"
            sys.exit(1)
        except Exception as e:
            print "ERROR: RPC fail: %s" % e
            sys.exit(1)

        # Check remote failure.
        if controller.Failed():
            print "ERROR: Remote fail: %s" % controller.ErrorText()
            sys.exit(1)

        return response

    def Step(self, msg):
        return self.doRequest(self.step, msg)

    def step(self, service, controller, args):
        msg = args[0]
        request = raft_server_pb2.StepRequest()
        request.message.CopyFrom(msg)
        response = service.Step(controller, request)

        return response

    def Status(self):
        return self.doRequest(self.status)

    def status(self, service, controller, args):
        request = raft_server_pb2.StatusRequest()
        response = service.Status(controller, request)

        return response


class RaftServerTest(unittest.TestCase):
    def test_LeaderElectionBootstrap(self):
        for server_num in range(1, 6):
            with LocalCluster(server_num) as cluster:
                time.sleep(3)

                status = [RaftClient(cluster.node(i).address()).Status() for i in range(0, server_num)]

                leader = status[0].leader
                for i in range(1, server_num):
                    self.assertEqual(leader, status[i].leader)

                # term must be 1
                for i in range(0, server_num):
                    self.assertEqual(1, status[i].raftTerm)

                # log index must be 1
                for i in range(0, server_num):
                    self.assertEqual(1, status[i].raftIndex)

    def test_LeaderDownReelection(self):
        for server_num in range(2, 6):
            with LocalCluster(server_num) as cluster:
                time.sleep(3)

                status = RaftClient(cluster.node(1).address()).Status()
                cluster.kill(status.leader)
                print status

                time.sleep(3)
                status = [RaftClient(cluster.node(i).address()).Status() for i in range(0, server_num)]
                leader = status[0].leader
                for i in range(1, server_num):
                    self.assertEqual(leader, status[i].leader)

                print status[0]


if __name__ == '__main__':
    unittest.main()
