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
#include "concurrentqueue/blockingconcurrentqueue.h"

#include <yaraft/raw_node.h>

namespace consensus {

// RaftTaskExecutor provides a task model for callers to submitting tasks to
// manipulate the RawNode(aka FSM), all in asynchronous way.
// The tasks may be submitted from RaftTimer (for ticking), RaftService::Step, ReplicatedLog::Write,
// currently they will all be executed sequentially, the underlying worker is a single thread.
//
class RaftTaskExecutor {
 public:
  explicit RaftTaskExecutor(yaraft::RawNode* node) : node_(node) {}

  typedef std::function<void(yaraft::RawNode* node)> RaftTask;

  void Submit(RaftTask task) {
    queue_.enqueue(task);
  }

  void Start();

  void Stop();

 private:
  yaraft::RawNode* node_;

  moodycamel::BlockingConcurrentQueue<RaftTask> queue_;

  BackgroundWorker worker_;
};

}  // namespace consensus