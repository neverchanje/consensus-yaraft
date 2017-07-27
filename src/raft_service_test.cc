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

#include "raft_service.h"
#include "raft_task_executor_test.h"

#include <sofa/pbrpc/closure.h>
#include <sofa/pbrpc/rpc_controller.h>

using namespace consensus;

class RaftServiceTest : public RaftTaskExecutorTest {
 public:
};

TEST_F(RaftServiceTest, Step) {
  yaraft::RawNode node(conf_);
  RaftTaskExecutor executor(&node, taskQueue_);
  RaftServiceImpl service(&executor);

  pb::StepResponse response;
  pb::StepRequest request;

  auto msg = new yaraft::pb::Message;
  request.set_allocated_message(msg);
  auto done = sofa::pbrpc::NewClosure([]() {});
  service.Step(nullptr, &request, &response, done);
  ASSERT_EQ(response.code(), pb::StepLocalMsg);

  msg = new yaraft::pb::Message;
  msg->set_from(111);
  msg->set_type(yaraft::pb::MsgHeartbeatResp);
  request.set_allocated_message(msg);
  done = sofa::pbrpc::NewClosure([]() {});
  service.Step(nullptr, &request, &response, done);
  ASSERT_EQ(response.code(), pb::StepPeerNotFound);
}