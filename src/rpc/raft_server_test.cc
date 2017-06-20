// Copyright 2017 Wu Tao
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "rpc/raft_client.h"

#include <gtest/gtest.h>
#include <yaraft/fluent_pb.h>

using namespace consensus::rpc;

// This test verifies that RaftServer will correctly handle error status returned by
// RawNode::Step.
TEST(RaftServer, StepErrorHandling) {
  typedef yaraft::PBMessage PBMsg;

  SyncRaftClient client;

  struct TestData {
    PBMsg msg;

    pb::StatusCode code;
  } tests[] = {
      {PBMsg(), pb::StepLocalMsg},
      {PBMsg().From(100).Type(yaraft::pb::MsgHeartbeatResp).v, pb::StepPeerNotFound},
  };

  for (auto t : tests) {
    pb::Response resp = client.Step("127.0.0.1:12321", new yaraft::pb::Message(t.msg.v));
    ASSERT_EQ(resp.code(), t.code);
  }
}