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

#include "base/background_worker.h"

#include "raft_task_executor.h"
#include "ready_flusher.h"
#include "replicated_log_impl.h"

#include <boost/thread/latch.hpp>

namespace consensus {

class ReadyFlusher::Impl {
 public:
  Impl() = default;

  void Register(ReplicatedLogImpl *log) {
    std::lock_guard<std::mutex> g(mu_);
    logs_.push_back(log);
  }

  void Start() {
    FATAL_NOT_OK(worker_.StartLoop(std::bind(&Impl::flushRound, this)),
                 "ReadyFlusher::Impl::Start");
  }

  void Stop() {
    FATAL_NOT_OK(worker_.Stop(), "ReadyFlusher::Impl::Stop");
  }

 private:
  void flushRound() {
    mu_.lock();
    std::vector<ReplicatedLogImpl *> logs = logs_;
    mu_.unlock();

    if (logs.empty()) {
      return;
    }

    for (auto rl : logs) {
      yaraft::Ready *rd = rl->executor_->GetReady();
      if (rd) {
        std::async(std::bind(&Impl::flushReady, this, rl, rd));
      }
    }
  }

  void flushReady(ReplicatedLogImpl *rl, yaraft::Ready *rd) {
    yaraft::pb::HardState *hs = nullptr;
    std::unique_ptr<yaraft::Ready> g(rd);

    // the leader can write to its disk in parallel with replicating to the followers and them
    // writing to their disks.
    // For more details, check raft thesis 10.2.1
    if (rd->currentLeader == rl->Id()) {
      if (!rd->messages.empty()) {
        rl->cluster_->Pass(rd->messages);
        rd->messages.clear();
      }
    }

    if (rd->hardState) {
      hs = rd->hardState.get();
    }

    if (!rd->entries.empty()) {
      FATAL_NOT_OK(rl->wal_->Write(rd->entries, hs), "Wal::Write");
    } else {
      FATAL_NOT_OK(rl->wal_->Write(hs), "Wal::Write");
    }

    // committedIndex has changed
    if (rd->hardState && rd->hardState->has_commit()) {
      rl->walCommitObserver_->Notify(rd->hardState->commit());
    }

    // states have already been persisted.
    rd->Advance(rl->memstore_);

    // followers should respond only after state persisted
    if (rd->currentLeader != rl->Id()) {
      if (!rd->messages.empty()) {
        rl->cluster_->Pass(rd->messages);
        rd->messages.clear();
      }
    }
  }

 private:
  std::vector<ReplicatedLogImpl *> logs_;
  std::mutex mu_;

  BackgroundWorker worker_;
};

void ReadyFlusher::Register(ReplicatedLogImpl *log) {
  impl_->Register(log);
}

ReadyFlusher::ReadyFlusher() : impl_(new Impl) {
  impl_->Start();
}

ReadyFlusher::~ReadyFlusher() {
  impl_->Stop();
}

}  // namespace consensus
