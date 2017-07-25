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
#include "base/logging.h"
#include "base/stl_container_utils.h"
#include "rpc/raft_client.h"

namespace consensus {
namespace rpc {

Peer::Peer(const std::string& url) : client_(new AsyncRaftClient(url)) {}

void Peer::AsyncSend(yaraft::pb::Message* msg) {
  client_->Step(msg);
}

Status PeerManager::Pass(std::vector<yaraft::pb::Message>& mails) {
  for (const auto& m : mails) {
    CHECK(m.to() != 0);
    CHECK(peerMap_.find(m.to()) != peerMap_.end());

    auto newMsg = new yaraft::pb::Message(std::move(m));
    peerMap_[m.to()]->AsyncSend(newMsg);
  }
  return Status::OK();
}

PeerManager::~PeerManager() {
  STLDeleteContainerPairSecondPointers(peerMap_.begin(), peerMap_.end());
}

}  // namespace rpc
}  // namespace consensus
