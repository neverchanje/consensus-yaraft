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

#include "base/status.h"
#include "rpc/pb/raft_server.pb.h"

#include <sofa/pbrpc/pbrpc.h>
#include <yaraft/pb/raftpb.pb.h>

namespace consensus {
namespace rpc {

class RaftServiceImpl;
class Peer {
 public:
  Peer(pb::RaftService* server, const std::string& url);

  Status AsyncSend(const yaraft::pb::Message& msg);

 private:
  void stepDoneCallback(const yaraft::pb::Message& response);

 private:
  sofa::pbrpc::RpcChannel channel_;
  sofa::pbrpc::RpcController controller_;

  RaftServiceImpl* server_;
};

}  // namespace rpc
}  // namespace consensus
