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

#include <thread>
#include <unordered_map>

#include "base/logging.h"
#include "config.h"
#include "rpc/pb/raft_server.pb.h"
#include "wal/wal.h"

#include <sofa/pbrpc/pbrpc.h>
#include <yaraft/yaraft.h>

namespace consensus {
namespace rpc {

class Peer;
class RaftTimer;
class RaftServiceImpl : public rpc::pb::RaftService {
 public:
  static RaftServiceImpl *New();

  virtual ~RaftServiceImpl();

  virtual void Serve(google::protobuf::RpcController *controller, const pb::Request *request,
                     pb::Response *response, google::protobuf::Closure *done) override;

  // @param `request` may be mutated after Step.
  void Step(sofa::pbrpc::RpcController *controller, const pb::Request *request,
            pb::Response *response);

  void Tick(google::protobuf::RpcController *controller, const pb::Request *request,
            pb::Response *response);

  // Stop the background raft timer.
  void Stop();

  // URL of this server.
  std::string Url() const {
    return url_;
  }

  std::string Name() const {
    return FLAGS_name;
  }

  uint64_t Id() const {
    return id_;
  }

 private:
  void handleReady(yaraft::Ready *rd);

  // helper function of `broadcast`.
  // broadcast mails in `rd` to their specified destination.
  // return false and warn for error if broadcast fails.
  bool broadcastAllReadyMails(yaraft::Ready *rd);

  // asynchronously broadcast all messages in `mails` to their specified destination.
  Status broadcast(std::vector<yaraft::pb::Message> &mails);

 private:
  friend class Peer;

  RaftServiceImpl() = default;

  std::unique_ptr<yaraft::RawNode> node_;
  std::unique_ptr<wal::WriteAheadLog> wal_;

  // name -> peer id
  std::unordered_map<std::string, uint64_t> peerNameToId_;

  // peer id -> Peer*
  std::vector<Peer *> peerMap_;

  std::string url_;
  uint64_t id_;

  yaraft::Storage *storage_;

  std::unique_ptr<RaftTimer> bg_timer_;
  std::thread bg_timer_thread_;
};

}  // namespace rpc
}  // namespace consensus
