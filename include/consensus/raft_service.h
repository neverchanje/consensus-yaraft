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

#pragma once

#include <consensus/pb/raft_server.pb.h>

namespace consensus {

class RaftTaskExecutor;

class RaftServiceImpl : public pb::RaftService {
 public:
  explicit RaftServiceImpl(RaftTaskExecutor *executor) : executor_(executor) {}

  ~RaftServiceImpl() = default;

  // RaftService::Step handles each request by calling RawNode::Step. If the request message
  // is invalid, the RaftService will respond with an error code.
  // @param `request` may be mutated after Step.
  void Step(google::protobuf::RpcController *controller, const pb::StepRequest *request,
            pb::StepResponse *response, google::protobuf::Closure *done) override;

  void Status(::google::protobuf::RpcController *controller, const pb::StatusRequest *request,
              pb::StatusResponse *response, ::google::protobuf::Closure *done) override;

 private:
  RaftTaskExecutor *executor_;
};

}  // namespace consensus
