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
#include "rpc/mock_cluster.h"
#include "wal/mock_wal.h"

#include "ready_flusher.h"
#include "wal_commit_observer.h"

using namespace consensus;

// This test verifies that ReadyFlusher will notify the wal-writer when the pending log
// has committed.
TEST(ReadyFlusherTest, Commit) {
  using yaraft::PBEntry;
  using yaraft::PBHardState;

  rpc::MockCluster cluster;
  wal::MockWriteAheadLog wal;

  WalCommitObserver observer;
  ReadyFlusher flusher(&observer, &cluster, &wal);
  flusher.Start();

  SimpleChannel<Status> chan;
  observer.Register(std::make_pair<uint64_t, uint64_t>(4, 6), &chan);

  // advance the committedIndex to 6
  yaraft::Ready rd;
  rd.hardState.reset(new yaraft::pb::HardState(PBHardState().Commit(6).v));
  flusher.ReadyGo(&rd);

  Status s;
  chan >>= s;
  ASSERT_OK(s);

  flusher.Stop();
}