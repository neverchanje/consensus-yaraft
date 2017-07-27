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

#include <map>
#include <memory>

#include "pb/raft_server.pb.h"

#include "base/slice.h"
#include "base/status.h"
#include "base/task_queue.h"

#include <silly/disallow_copying.h>
#include <yaraft/yaraft.h>

namespace consensus {

struct ReplicatedLogOptions {
  std::string wal_dir;

  // id -> IP
  std::map<uint64_t, std::string> initial_cluster;

  uint64_t id;

  // time (in milliseconds) of a heartbeat interval.
  uint32_t heartbeat_interval;

  // time (in milliseconds) for an election to timeout.
  uint32_t election_timeout;

  // dedicated worker of the raft node.
  // there may have multiple instances sharing the same queue.
  TaskQueue* taskQueue;
};

// A ReplicatedLog is a distributed log storage with strong consistency. Every single
// write will be replicated to a majority of nodes if successes.
// If the current node is not leader, the write fails immediately.
//
// Not Thread Safe
class ReplicatedLog {
  __DISALLOW_COPYING__(ReplicatedLog);

 public:
  static StatusWith<ReplicatedLog*> New(const ReplicatedLogOptions&);

  // Returns WalWriteToNonLeader if the current node is not leader.
  Status Write(const Slice& log);

  // TODO: Status AsyncWrite(const Slice& log);

  // the life ownership of RaftService is still kept in ReplicatedLog.
  pb::RaftService* RaftServiceInstance();

  yaraft::RaftInfo GetInfo();

  ~ReplicatedLog();

 private:
  ReplicatedLog() = default;

 private:
  friend class ReplicatedLogTest;

  class Impl;
  friend class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace consensus
