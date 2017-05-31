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

#include "rpc/raft_server.h"
#include "base/stl_container_utils.h"
#include "rpc/peer.h"

namespace consensus {
namespace rpc {

// asynchronously broadcast all messages in `mails`.
Status RaftServiceImpl::broadcast(const std::vector<yaraft::pb::Message> &mails) {
  for (const auto &m : mails) {
    CHECK(peerMap_.find(m.to()) != peerMap_.end());
    RETURN_NOT_OK(peerMap_[m.to()]->AsyncSend(m));
  }
  return Status::OK();
}

RaftServiceImpl::~RaftServiceImpl() {
  STLDeleteContainerPairSecondPointers(peerMap_.begin(), peerMap_.end());
}

}  // namespace rpc
}  // namespace consensus

int main() {
  ::sofa::pbrpc::RpcServerOptions opts;
  ::sofa::pbrpc::RpcServer rpcServer(opts);

  // Start rpc server.
  if (!rpcServer.Start("0.0.0.0:12321")) {
    LOG(ERROR) << "start rpc failed";
    return EXIT_FAILURE;
  }

  // Register service.
  auto raftService = new consensus::rpc::RaftServiceImpl();
  if (!rpcServer.RegisterService(raftService)) {
    FMT_LOG(ERROR, "export service failed");
    return EXIT_FAILURE;
  }

  // Wait signal.
  rpcServer.Run();

  // Stop rpc rpc.
  rpcServer.Stop();

  return 0;
}
