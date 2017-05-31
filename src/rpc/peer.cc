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

#include "rpc/peer.h"
#include "rpc/raft_server.h"

namespace consensus {
namespace rpc {

static sofa::pbrpc::RpcClient* getGlobalClient() {
  static sofa::pbrpc::RpcClient globalClient;
  return &globalClient;
}

void Peer::stepDoneCallback(const yaraft::pb::Message& response) {
  server_->handle(response, nullptr);
}

Peer::Peer(pb::RaftService* server, const std::string& url) : channel_(getGlobalClient(), url) {
  controller_.SetTimeout(3000);

  server_ = static_cast<RaftServiceImpl*>(server);
}

Status Peer::AsyncSend(const yaraft::pb::Message& msg) {
  pb::Request request;
  request.set_allocated_message(new yaraft::pb::Message(msg));
  pb::Response response;

  auto done = google::protobuf::NewCallback<Peer, const yaraft::pb::Message&>(
      this, &Peer::stepDoneCallback, response.message());

  pb::RaftService_Stub* stub = new pb::RaftService_Stub(&channel_);
  stub->Step(&controller_, &request, &response, done);

  return Status::OK();
}

}  // namespace rpc
}  // namespace consensus
