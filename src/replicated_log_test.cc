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

#include "base/testing.h"

#include "replicated_log_impl.h"

using namespace consensus;

class ReplicatedLogTest : public BaseTest {
 public:
  void SetUp() override {
    guard = CreateTestDirGuard();

    options.initial_cluster[1] = "127.0.0.1:12321";
    options.id = 1;
    options.wal_dir = GetTestDir();
    options.heartbeat_interval = 100;
    options.election_timeout = 1000;
    options.taskQueue = new TaskQueue;
  }

  void TearDown() override {
    delete replicatedLog;
    delete guard;
    delete options.taskQueue;
  }

 protected:
  ReplicatedLog* replicatedLog;
  ReplicatedLogOptions options;
  TestDirectoryGuard* guard;
};

TEST_F(ReplicatedLogTest, WriteToNonLeader) {
  ASSIGN_IF_ASSERT_OK(ReplicatedLog::New(options), replicatedLog);

  Status s = replicatedLog->Write("abc");
  ASSERT_EQ(s.Code(), Error::WalWriteToNonLeader);
}

TEST_F(ReplicatedLogTest, WriteLeader) {
  ASSIGN_IF_ASSERT_OK(ReplicatedLog::New(options), replicatedLog);

  while (replicatedLog->GetInfo().currentTerm != 1) {
    sleep(1);
  }

  for (int i = 0; i < 4; i++) {
    Status s = replicatedLog->Write("abc");
    ASSERT_OK(s);
  }

  ASSERT_EQ(replicatedLog->GetInfo().logIndex, 5);
  ASSERT_EQ(replicatedLog->GetInfo().commitIndex, 5);
  ASSERT_EQ(replicatedLog->GetInfo().currentTerm, 1);
  ASSERT_EQ(replicatedLog->GetInfo().currentLeader, 1);
}