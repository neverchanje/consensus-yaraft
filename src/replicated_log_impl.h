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
  static StatusWith<ReplicatedLog *> New(const ReplicatedLogOptions &oldOptions) {
    // The construction order is:
    // - RawNode
    // - RaftTaskExecutor (depends on RawNode)
    // - RaftTimer, (depends on RaftTaskExecutor)
    // - ReadyFlusher (depends on WalCommitObserver, WAL, RPC)
    // - ReplicatedLog
    ReplicatedLogOptions options = oldOptions;
    RETURN_NOT_OK(options.Validate());

    auto impl = new ReplicatedLogImpl;

    // -- RawNode --
    auto conf = new yaraft::Config;
    conf->electionTick = options.election_timeout;
    conf->heartbeatTick = options.heartbeat_interval;
    conf->id = options.id;
    if (!options.memstore) {
      options.memstore = new yaraft::MemoryStorage;
    }
    conf->storage = options.memstore;
    for (const auto &e : options.initial_cluster) {
      conf->peers.push_back(e.first);
    }
    impl->node_.reset(new yaraft::RawNode(conf));

    // -- RaftTaskExecutor --
    TaskQueue *taskQueue = options.taskQueue;
    if (!taskQueue) {
      taskQueue = new TaskQueue;
    }
    impl->executor_.reset(new RaftTaskExecutor(impl->node_.get(), taskQueue));

    // -- RaftTimer --
    impl->timer_.reset(options.timer);
    if (!impl->timer_) {
      impl->timer_.reset(new RaftTimer);
    }
    impl->timer_->Register(impl->executor_.get());

    // -- ReadyFlusher --
    impl->wal_ = options.wal;
    impl->walCommitObserver_.reset(new WalCommitObserver);
    impl->memstore_ = options.memstore;
    impl->cluster_.reset(rpc::Cluster::Default(options.initial_cluster));
    impl->flusher_.reset(options.flusher);
    if (!impl->flusher_) {
      impl->flusher_.reset(new ReadyFlusher);
    }
    impl->flusher_->Register(impl);

    auto rl = new ReplicatedLog;
    rl->impl_.reset(impl);
    return rl;
  }

  ~ReplicatedLogImpl() = default;

  SimpleChannel<Status> AsyncWrite(const Slice &log) {
    SimpleChannel<Status> channel;

    executor_->Submit([&](yaraft::RawNode *node) {
      uint64_t id = Id();
      if (node->IsLeader()) {
        channel <<=
            FMT_Status(WalWriteToNonLeader, "writing to a non-leader node, [id: {}, leader: {}]",
                       id, node->LeaderHint());
        return;
      }

      uint64_t newIndex = node->LastIndex() + 1;

      yaraft::Status s = node->Propose(log);
      if (UNLIKELY(!s.IsOK())) {
        channel <<= Status::Make(Error::YARaftError, s.ToString());
        return;
      }

      // listening for the committedIndex to forward to the newly-appended log.
      walCommitObserver_->Register(std::make_pair(newIndex, newIndex), &channel);
    });

    return channel;
  }

  uint64_t Id() const {
    return node_->Id();
  }

 private:
  friend class ReadyFlusher;

  std::unique_ptr<yaraft::RawNode> node_;

  std::unique_ptr<RaftTaskExecutor> executor_;

  std::unique_ptr<WalCommitObserver> walCommitObserver_;

  std::unique_ptr<rpc::Cluster> cluster_;

  std::shared_ptr<RaftTimer> timer_;

  std::shared_ptr<ReadyFlusher> flusher_;

  yaraft::MemoryStorage *memstore_;

  wal::WriteAheadLog *wal_;
};

}  // namespace consensus