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
#include "rpc/pb/raft_server.pb.h"

#include <sofa/pbrpc/pbrpc.h>

namespace consensus {
namespace rpc {

class SyncRaftClient : sofa::pbrpc::RpcClient {
 public:
  explicit SyncRaftClient(const std::string& url)
      : channel_(this, url, sofa::pbrpc::RpcChannelOptions()) {}

  // Synchronously sending request to specified url.
  // NOTE: The ownership of `msg` will be taken over by this method.
  StatusWith<pb::StepResponse> Step(yaraft::pb::Message* msg) {
    rpc::pb::StepRequest request;
    request.set_allocated_message(msg);

    return sendRequest(request);
  }

  StatusWith<pb::TickResponse> Tick(uint32_t ticks) {
    rpc::pb::TickRequest request;
    request.set_ticks(ticks);

    return sendRequest(request);
  }

 private:
  StatusWith<pb::StepResponse> sendRequest(rpc::pb::StepRequest& request) {
    sofa::pbrpc::RpcController cntl;
    cntl.SetTimeout(3000);

    rpc::pb::StepResponse response;
    rpc::pb::RaftService_Stub stub(&channel_);
    stub.Step(&cntl, &request, &response, NULL);

    if (cntl.Failed()) {
      auto errStr = fmt::format("request failed: {}", cntl.ErrorText());
      LOG(ERROR) << errStr;
      return Status::Make(Error::RpcError, errStr);
    }
    return response;
  }

  StatusWith<pb::TickResponse> sendRequest(rpc::pb::TickRequest& request) {
    sofa::pbrpc::RpcController cntl;
    cntl.SetTimeout(3000);

    rpc::pb::TickResponse response;
    rpc::pb::RaftService_Stub stub(&channel_);
    stub.Tick(&cntl, &request, &response, NULL);

    if (cntl.Failed()) {
      auto errStr = fmt::format("request failed: {}", cntl.ErrorText());
      LOG(ERROR) << errStr;
      return Status::Make(Error::RpcError, errStr);
    }
    return response;
  }

 private:
  sofa::pbrpc::RpcChannel channel_;
};

class AsyncRaftClient : sofa::pbrpc::RpcClient {
 public:
  explicit AsyncRaftClient(const std::string& url)
      : channel_(this, url, sofa::pbrpc::RpcChannelOptions()) {}

  // Asynchronously sending request to specified url.
  void Step(yaraft::pb::Message* msg) {
    // -- prepare parameters --

    auto cntl = new sofa::pbrpc::RpcController();
    cntl->SetTimeout(3000);

    auto request = new rpc::pb::StepRequest();
    request->set_allocated_message(msg);

    auto response = new rpc::pb::StepResponse();

    auto done =
        sofa::pbrpc::NewClosure(this, &AsyncRaftClient::doneCallBack, cntl, request, response);

    // -- request --

    rpc::pb::RaftService_Stub stub(&channel_);
    stub.Step(cntl, request, response, done);
  }

  // `onSuccess` should not destroy the response object.
  void RegisterOnSuccess(std::function<void(rpc::pb::StepResponse*)> onSuccess) {
    onSuccess_ = onSuccess;
  }

  void RegisterOnFail(std::function<void()> onFail) {
    onFail_ = onFail;
  }

 private:
  void doneCallBack(const sofa::pbrpc::RpcController* cntl, rpc::pb::StepRequest* request,
                    rpc::pb::StepResponse* response) {
    if (cntl->Failed()) {
      FMT_SLOG(ERROR, "request failed: %s", cntl->ErrorText().c_str());

      if (onFail_) {
        onFail_();
      }
    } else {
      if (onSuccess_) {
        onSuccess_(response);
      }
      delete response;
    }
    delete cntl;
    delete request;
  }

 private:
  sofa::pbrpc::RpcChannel channel_;
  std::function<void(rpc::pb::StepResponse*)> onSuccess_;
  std::function<void()> onFail_;
};

}  // namespace rpc
}  // namespace consensus