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

#include "base/background_worker.h"
#include "base/simple_channel.h"
#include "base/status.h"
#include "readerwriterqueue/readerwriterqueue.h"
#include "rpc/cluster.h"
#include "wal/wal.h"

#include <wal/wal.h>
#include <yaraft/ready.h>

namespace consensus {

// ReadyFlusher is a single background thread for asynchronously flushing the Ready-s,
// so that the FSM thread can be free from stalls every time when it generates a Ready.

class WalCommitObserver;
class RaftTaskExecutor;
class ReadyFlusher {
 public:
  ReadyFlusher(WalCommitObserver* observer, rpc::Cluster* peers, wal::WriteAheadLog* wal,
               RaftTaskExecutor* executor, yaraft::MemoryStorage* memstore)
      : walCommitObserver_(observer),
        peers_(peers),
        wal_(wal),
        executor_(executor),
        memstore_(memstore) {}

  void Start();

  void Stop();

 private:
  Status flushReady(yaraft::Ready* rd);

  void onReadyFlushed(yaraft::Ready* rd);

 private:
  WalCommitObserver* walCommitObserver_;
  BackgroundWorker worker_;
  RaftTaskExecutor* executor_;
  rpc::Cluster* peers_;
  wal::WriteAheadLog* wal_;
  yaraft::MemoryStorage* memstore_;
};

}  // namespace consensus
