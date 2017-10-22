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

#include "base/simple_channel.h"
#include "raft_task_executor_test.h"
#include "raft_timer.h"

using namespace consensus;

class RaftTimerTest : public RaftTaskExecutorTest {};

// This test verifies that RaftTimer sends ticks regularly to RaftTaskExecutor, which
// is supposed to start election after election timeout.
TEST_F(RaftTimerTest, Timeout) {
  conf_->peers = {1};
  conf_->electionTick = 200;
  yaraft::RawNode node(conf_);
  RaftTaskExecutor executor(&node, taskQueue_);
  RaftTimer timer;
  timer.Register(&executor);

  sleep(2);

  uint64_t currentTerm = 0;
  uint64_t lastIndex = 0;

  Barrier barrier;
  executor.Submit([&](yaraft::RawNode *n) {
    currentTerm = n->CurrentTerm();
    lastIndex = n->LastIndex();

    barrier.Signal();
  });
  barrier.Wait();

  ASSERT_GE(currentTerm, 1);
  ASSERT_GE(lastIndex, 1);
}