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
#include "rpc/raft_timer.h"

#include <base/env.h>
#include <boost/algorithm/string/trim.hpp>

namespace consensus {
namespace rpc {

RaftServiceImpl::~RaftServiceImpl() {
  STLDeleteContainerPointers(peerMap_.begin(), peerMap_.end());
}

Status RaftServiceImpl::broadcast(std::vector<yaraft::pb::Message> &mails) {
  for (const auto &m : mails) {
    CHECK(peerMap_.size() < m.to() && m.to() != 0);

    auto newMsg = new yaraft::pb::Message(std::move(m));
    peerMap_[m.to()]->AsyncSend(newMsg);
  }
  return Status::OK();
}

bool RaftServiceImpl::broadcastAllReadyMails(yaraft::Ready *rd) {
  auto st = broadcast(rd->messages);
  if (!st.IsOK()) {
    LOG(WARNING) << "RaftServiceImpl::broadcast: " << st;
  }
  rd->messages.clear();
  return st.IsOK();
}

static void printFlags() {
  FMT_LOG(INFO, "--wal_dir=\"{}\"", FLAGS_wal_dir);
  FMT_LOG(INFO, "--name=\"{}\"", FLAGS_name);
  FMT_LOG(INFO, "--heartbeat_interval={}ms", FLAGS_heartbeat_interval);
  FMT_LOG(INFO, "--election_timeout={}ms", FLAGS_election_timeout);
  FMT_LOG(INFO, "--initial_cluster=\"{}\"", FLAGS_initial_cluster);
}

RaftServiceImpl *RaftServiceImpl::New() {
  printFlags();

  FATAL_NOT_OK(Env::Default()->CreateDirIfMissing(FLAGS_wal_dir),
               fmt::format("create wal_dir=\"{}\" if missing", FLAGS_wal_dir));
  wal::WriteAheadLog *wal;
  auto memstore = new ::yaraft::MemoryStorage;
  FATAL_NOT_OK(wal::WriteAheadLog::Default(FLAGS_wal_dir, &wal, memstore),
               fmt::format("initialize data from wal_dir=\"{}\"", FLAGS_wal_dir));

  auto conf = new yaraft::Config;
  conf->storage = memstore;
  conf->electionTick = FLAGS_election_timeout;
  conf->heartbeatTick = FLAGS_heartbeat_interval;

  std::map<std::string, std::string> peerNameToIp;
  FATAL_NOT_OK(ParseClusterMembershipFromGFlags(&peerNameToIp), "--initial_cluster parse failed");
  if (peerNameToIp.find(FLAGS_name) == peerNameToIp.end()) {
    FMT_LOG(FATAL, "couldn't find local name \"{}\" in the initial cluster configuration",
            FLAGS_name);
  }

  for (uint64_t i = 1; i <= peerNameToIp.size(); i++) {
    conf->peers.push_back(i);
  }

  auto server = new RaftServiceImpl();
  server->peerMap_.reserve(peerNameToIp.size());
  uint64_t id = 0;
  for (auto &e : peerNameToIp) {
    ++id;

    auto it = server->peerNameToId_.find(e.first);
    CHECK(it == server->peerNameToId_.end());
    server->peerNameToId_[e.first] = id;

    // there's no need to create a connection to itself.
    if (FLAGS_name != e.first) {
      FMT_SLOG(INFO, "New peer {name: \"%s\", url: \"%s\"} added.", e.first, e.second);
      server->peerMap_[id] = new Peer(server, e.second);
    }
  }

  server->url_ = peerNameToIp[FLAGS_name];
  server->id_ = conf->id = server->peerNameToId_[FLAGS_name];

  auto node = new yaraft::RawNode(conf);
  server->node_.reset(node);
  server->wal_.reset(wal);
  server->storage_ = memstore;

  server->bg_timer_thread_ = std::thread(
      [](RaftServiceImpl *s) {
        s->bg_timer_ = std::unique_ptr<RaftTimer>(new RaftTimer(s));
        s->bg_timer_->Run();
      },
      server);

  return server;
}

void RaftServiceImpl::Serve(google::protobuf::RpcController *controller, const pb::Request *request,
                            pb::Response *response, google::protobuf::Closure *done) {
  sofa::pbrpc::RpcController *cntl = static_cast<sofa::pbrpc::RpcController *>(controller);

  if (request->ticks() == 0) {
    Step(cntl, request, response);
  } else {
    Tick(cntl, request, response);
  }

  done->Run();
}

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

void RaftServiceImpl::Step(sofa::pbrpc::RpcController *cntl, const pb::Request *request,
                           pb::Response *response) {
  FMT_LOG(INFO, "RaftService::Step(): message from {}: {}", cntl->RemoteAddress(),
          yaraft::DumpPB(request->message()));

  auto s = node_->Step(const_cast<::yaraft::pb::Message &>(request->message()));
  if (UNLIKELY(!s.IsOK())) {
    LOG(ERROR) << "RawNode::Step: " << s.ToString();
    response->set_code(yaraftErrorCodeToRpcStatusCode(s.Code()));
    return;
  }

  handleReady(node_->GetReady());
}

void RaftServiceImpl::Tick(google::protobuf::RpcController *cntl, const pb::Request *request,
                           pb::Response *response) {
  for (uint32_t i = 0; i < request->ticks(); i++) {
    node_->Tick();
  }

  handleReady(node_->GetReady());
}

void RaftServiceImpl::handleReady(yaraft::Ready *rd) {
  using namespace yaraft::pb;
  if (rd == nullptr) {
    return;
  }

  if (!rd->entries->empty()) {
    FATAL_NOT_OK(wal_->AppendEntries(*rd->entries), "WriteAheadLog::AppendEntries");
    node_->Advance(*rd);
  }

  if (!rd->hardState) {
  }

  if (!rd->messages.empty()) {
    broadcastAllReadyMails(rd);
  }
}

void RaftServiceImpl::Stop() {
  bg_timer_->Stop();
  bg_timer_thread_.join();
}

}  // namespace rpc
}  // namespace consensus
