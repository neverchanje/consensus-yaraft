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

#include <thread>

#include "raft_service.h"
#include "raft_task_executor.h"
#include "replicated_log.h"
#include "replicated_log_impl.h"

namespace consensus {

Status ReplicatedLog::Write(const Slice &log) {
  Status s;
  SimpleChannel<Status> chan = impl_->AsyncWrite(log);
  chan >>= s;
  return s;
}

StatusWith<ReplicatedLog *> ReplicatedLog::New(const ReplicatedLogOptions &options) {
  return ReplicatedLogImpl::New(options);
}

RaftTaskExecutor *ReplicatedLog::RaftTaskExecutorInstance() const {
  return impl_->executor_.get();
}

ReplicatedLog::~ReplicatedLog() {}

yaraft::RaftInfo ReplicatedLog::GetInfo() {
  Barrier barrier;
  yaraft::RaftInfo info;

  impl_->executor_->Submit([&](yaraft::RawNode *node) {
    info = node->GetInfo();
    barrier.Signal();
  });
  barrier.Wait();

  return info;
}

SimpleChannel<Status> ReplicatedLog::AsyncWrite(const Slice &log) {
  return impl_->AsyncWrite(log);
}

Status ReplicatedLogOptions::Validate() const {
#define ConfigNotNull(var) \
  if ((var) == nullptr)    \
    return FMT_Status(BadConfig, "ReplicatedLogOptions::" #var " should not be null");
  ConfigNotNull(taskQueue);
  ConfigNotNull(timer);
  ConfigNotNull(flusher);
  ConfigNotNull(memstore);
  ConfigNotNull(wal);
  return Status::OK();
}

ReplicatedLogOptions::ReplicatedLogOptions()
    : heartbeat_interval(100),
      election_timeout(10 * 1000),
      taskQueue(nullptr),
      flusher(nullptr),
      timer(nullptr),
      wal(nullptr),
      memstore(nullptr) {}

}  // namespace consensus
