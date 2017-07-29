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

#include "base/env.h"
#include "base/logging.h"
#include "rpc/peer.h"
#include "wal/wal.h"

#include "raft_service.h"
#include "raft_task_executor.h"
#include "raft_timer.h"
#include "ready_flusher.h"
#include "replicated_log.h"
#include "wal_commit_observer.h"

#include <yaraft/conf.h>

namespace consensus {

class ReplicatedLogImpl {
  friend class ReplicatedLog;

 public:
  static StatusWith<ReplicatedLog *> New(const ReplicatedLogOptions &options) {
    auto impl = new ReplicatedLogImpl;

    impl->timer_ = options.timer;
    impl->flusher_ = options.flusher;

    wal::WriteAheadLog *wal;
    auto memstore = new yaraft::MemoryStorage;
    FATAL_NOT_OK(Env::Default()->CreateDirIfMissing(options.wal_dir),
                 fmt::format("CreateDirIfMissing({})", options.wal_dir));
    FATAL_NOT_OK(wal::WriteAheadLog::Default(options.wal_dir, &wal, memstore),
                 "WriteAheadLog::Default");
    impl->wal_.reset(wal);
    impl->memstore_ = memstore;

    impl->cluster_.reset(rpc::Cluster::Default(options.initial_cluster));
    impl->walCommitObserver_.reset(new WalCommitObserver);

    auto conf = new yaraft::Config;
    conf->electionTick = options.election_timeout;
    conf->heartbeatTick = options.heartbeat_interval;
    conf->id = options.id;
    conf->storage = memstore;
    for (const auto &e : options.initial_cluster) {
      conf->peers.push_back(e.first);
    }

    impl->node_.reset(new yaraft::RawNode(conf));
    impl->executor_.reset(new RaftTaskExecutor(impl->node_.get(), options.taskQueue));
    impl->raftService_.reset(new RaftServiceImpl(impl->executor_.get()));

    impl->id_ = options.id;

    impl->timer_->Register(impl->executor_.get());
    impl->flusher_->Register(impl);

    auto rl = new ReplicatedLog;
    rl->impl_.reset(impl);
    return rl;
  }

  ~ReplicatedLogImpl() {}

  Status Write(const Slice &log) {
    SimpleChannel<Status> channel;

    executor_->Submit([&](yaraft::RawNode *node) {
      auto info = node->GetInfo();
      if (info.currentLeader != id_) {
        channel <<=
            FMT_Status(WalWriteToNonLeader, "writing to a non-leader node, [id: {}, leader: {}]",
                       id_, info.currentLeader);
        return;
      }

      uint64_t newIndex = info.logIndex + 1;

      yaraft::Status s = node->Propose(log);
      if (UNLIKELY(!s.IsOK())) {
        channel <<= Status::Make(Error::YARaftError, s.ToString());
        return;
      }

      // listening for the committedIndex to forward to the newly-appended log.
      walCommitObserver_->Register(std::make_pair(newIndex, newIndex), &channel);
    });

    Status s;
    channel >>= s;

    return s;
  }

 private:
  friend class ReadyFlusher;

  std::unique_ptr<yaraft::RawNode> node_;

  std::unique_ptr<wal::WriteAheadLog> wal_;

  std::unique_ptr<RaftTaskExecutor> executor_;

  std::unique_ptr<WalCommitObserver> walCommitObserver_;

  std::unique_ptr<rpc::Cluster> cluster_;

  std::unique_ptr<pb::RaftService> raftService_;

  RaftTimer *timer_;

  ReadyFlusher *flusher_;

  yaraft::MemoryStorage *memstore_;

  uint64_t id_;
};

}  // namespace consensus