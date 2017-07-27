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

#include "raft_task_executor_test.h"
#include "base/simple_channel.h"

#include <boost/thread.hpp>

using namespace consensus;

// This test verifies the tasks submitted to RaftTaskExecutor will be executed sequentially.
TEST_F(RaftTaskExecutorTest, TasksInSequence) {
  yaraft::RawNode node(conf_);
  RaftTaskExecutor executor(&node, taskQueue_);

  boost::thread_group group;
  boost::barrier barrier(3);

  // appending a character into std::string is absolutely a non-atomic operation,.
  std::string s;
  std::atomic_int count(0);

  SimpleChannel<void> chan;
  for (int i = 0; i < 3; i++) {
    group.create_thread([&]() {
      barrier.wait();

      for (int k = 0; k < 100; k++) {
        executor.Submit([&](yaraft::RawNode *n) {
          s.push_back('a');
          if (++count == 300) {
            chan.Signal();
          }
        });
      }
    });
  }
  chan.Wait();

  ASSERT_EQ(s.length(), 300);
  ASSERT_EQ(s, std::string(300, 'a'));
}