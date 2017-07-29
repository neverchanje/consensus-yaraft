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
  return impl_->Write(log);
}

StatusWith<ReplicatedLog *> ReplicatedLog::New(const ReplicatedLogOptions &options) {
  return ReplicatedLogImpl::New(options);
}

pb::RaftService *ReplicatedLog::RaftServiceInstance() {
  return impl_->raftService_.get();
}

ReplicatedLog::~ReplicatedLog() {}

yaraft::RaftInfo ReplicatedLog::GetInfo() {
  SimpleChannel<void> chan;
  yaraft::RaftInfo info;

  impl_->executor_->Submit([&](yaraft::RawNode *node) {
    info = node->GetInfo();
    chan.Signal();
  });
  chan.Wait();

  return info;
}

}  // namespace consensus
