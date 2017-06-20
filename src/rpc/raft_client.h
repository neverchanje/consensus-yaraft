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

#include "base/logging.h"
#include "rpc/pb/raft_server.pb.h"

#include <sofa/pbrpc/pbrpc.h>

namespace consensus {
namespace rpc {

class SyncRaftClient : sofa::pbrpc::RpcClient {
 public:
  SyncRaftClient() {}

  // Synchronously sending request to specified url.
  pb::Response Step(const std::string& url, yaraft::pb::Message* msg) {
    sofa::pbrpc::RpcChannelOptions channel_options;
    sofa::pbrpc::RpcChannel rpc_channel(this, url, channel_options);

    sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
    cntl->SetTimeout(3000);

    rpc::pb::Response response;
    rpc::pb::Request request;
    request.set_allocated_message(msg);

    rpc::pb::RaftService_Stub stub(&rpc_channel);
    stub.Step(cntl, &request, &response, NULL);

    if (cntl->Failed()) {
      FMT_SLOG(ERROR, "request failed: %s", cntl->ErrorText().c_str());
    } else {
      FMT_SLOG(INFO, "request succeed with response: {%s}", response.ShortDebugString());
    }

    delete cntl;
    return response;
  }
};

class AsyncRaftClient : sofa::pbrpc::RpcClient {
 public:
  AsyncRaftClient() {}

  static void doneCallBack(const sofa::pbrpc::RpcController* cntl, rpc::pb::Response& response) {
    if (cntl->Failed()) {
      FMT_SLOG(ERROR, "request failed: %s", cntl->ErrorText().c_str());
    } else {
      FMT_SLOG(INFO, "request succeed with response: {%s}", response.ShortDebugString());
    }
  }

  // Asynchronously sending request to specified url.
  void Step(const std::string& url, yaraft::pb::Message* msg) {
    sofa::pbrpc::RpcChannelOptions channel_options;
    sofa::pbrpc::RpcChannel rpc_channel(this, url, channel_options);

    sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
    cntl->SetTimeout(3000);

    auto request = new rpc::pb::Request();
    request->set_allocated_message(msg);
    rpc::pb::RaftService_Stub stub(&rpc_channel);

    auto done = sofa::pbrpc::NewClosure(&AsyncRaftClient::doneCallBack, cntl, response_);
    stub.Step(cntl, request, &response_, done);

    delete cntl;
    delete request;
  }

 private:
  pb::Response response_;
};

}  // namespace rpc
}  // namespace consensus