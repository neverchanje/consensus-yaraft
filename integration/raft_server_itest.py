import subprocess
import sys
import time
from sofa.pbrpc import client
import raft_server_pb2, yaraft.pb.raftpb_pb2
import unittest


class RaftNode:
    def __init__(self, id):
        self.id = id

    def name(self):
        return "infra{}".format(self.id)

    def address(self):
        return "127.0.0.1:{}".format(self.id + 12320)

    def wal_dir(self):
        return "{}.consensus".format(self.name())


class LocalCluster:
    def __init__(self, num):
        self.p = []
        self.server_num = num
        self.out = []
        self.nodes = [RaftNode(i + 1) for i in range(0, self.server_num)]

    # set up `server_num` number of servers.
    def __enter__(self):
        nodes = self.nodes
        addresses = ["{}={}".format(nodes[i].name(), nodes[i].address()) for i in range(0, self.server_num)]
        initial_cluster = "--initial_cluster=" + ';'.join(addresses)

        for i in range(0, self.server_num):
            wal_dir = "--wal_dir={}".format(nodes[i].wal_dir())
            name = "--name={}".format(nodes[i].name())
            out = open("server{}.log".format(i + 1), "w")

            self.p.append(subprocess.Popen(["../output/bin/raft_server", initial_cluster, wal_dir, name], stderr=out))
            self.out.append(out)

        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        for i in range(0, self.server_num):
            self.p[i].kill()
            self.out[i].close()

    def node(self, id):
        return self.nodes[id - 1]



class RaftClient:
    def __init__(self, url):
        self.channel = client.Channel(url)

    def request(self, msg):
        service = raft_server_pb2.RaftService_Stub(self.channel)
        controller = client.Controller()
        controller.SetTimeout(1.5)

        request = raft_server_pb2.Request()
        request.message.CopyFrom(msg)

        try:
            response = service.Serve(controller, request)
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

        # OK, print response.
        print "Response:\n\n%s" % response


class RaftServerTest(unittest.TestCase):
    def test_WriteAndRead(self):


if __name__ == '__main__':
    with LocalCluster(3) as cluster:
        time.sleep(5)

        msg = yaraft.pb.raftpb_pb2.Message()
        msg.type = yaraft.pb.raftpb_pb2.MsgProp
        msg.to = 1
        msg.term = 1
        entry = msg.entries.add()
        entry.Data = "abc"

        RaftClient(cluster.node(1).address()).request(msg)
