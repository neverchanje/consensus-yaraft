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
#include "raft_task_executor.h"
#include "raft_timer.h"

#include "base/logging.h"
#include "base/simple_channel.h"

#include <sofa/pbrpc/pbrpc.h>
#include <yaraft/pb_utils.h>

namespace consensus {

static pb::StatusCode yaraftErrorCodeToRpcStatusCode(yaraft::Error::ErrorCodes code) {
  switch (code) {
    case yaraft::Error::StepLocalMsg:
      return pb::StepLocalMsg;
    case yaraft::Error::StepPeerNotFound:
      return pb::StepPeerNotFound;
    default:
      LOG(FATAL) << "Unexpected error code: " << yaraft::Error::ToString(code);
      return pb::OK;
  }
}

void RaftServiceImpl::Step(google::protobuf::RpcController *controller,
                           const pb::StepRequest *request, pb::StepResponse *response,
                           google::protobuf::Closure *done) {
  FMT_LOG(INFO, "RaftService::Step(): {}", yaraft::DumpPB(request->message()));

  yaraft::pb::Message &msg = *const_cast<pb::StepRequest *>(request)->mutable_message();

  response->set_code(pb::OK);

  SimpleChannel<void> channel;
  executor_->Submit(std::bind(
      [&](yaraft::RawNode *node) {
        auto s = node->Step(msg);
        if (UNLIKELY(!s.IsOK())) {
          response->set_code(yaraftErrorCodeToRpcStatusCode(s.Code()));
        }
        channel.Signal();
      },
      std::placeholders::_1));
  channel.Wait();

  done->Run();
}

}  // namespace consensus
