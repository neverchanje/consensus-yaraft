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

#include "base/logging.h"
#include "pb/raft_server.pb.h"

#include <brpc/channel.h>

namespace consensus {
namespace rpc {

static void doneCallBack(pb::StepResponse* response, brpc::Controller* cntl) {
  if (cntl->Failed()) {
    FMT_SLOG(ERROR, "request failed: %s", cntl->ErrorText().c_str());
  } else {
    delete response;
  }
  delete cntl;
}

class AsyncRaftClient {
 public:
  explicit AsyncRaftClient(const std::string& url) {
    brpc::ChannelOptions options;
    options.max_retry = 0;  // no retry
    options.connect_timeout_ms = 2000;
    channel_.Init(url.c_str(), &options);
  }

  // Asynchronously sending request to specified url.
  void Step(yaraft::pb::Message* msg) {
    // -- prepare parameters --

    auto cntl = new brpc::Controller;
    cntl->set_timeout_ms(3000);

    pb::StepRequest request;
    request.set_allocated_message(msg);
    auto response = new pb::StepResponse;

    // -- request --

    pb::RaftService_Stub stub(&channel_);
    stub.Step(cntl, &request, response, brpc::NewCallback(&doneCallBack, response, cntl));
  }

 private:
  brpc::Channel channel_;
};

}  // namespace rpc
}  // namespace consensus