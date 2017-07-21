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

#include "ready_flusher.h"
#include "base/logging.h"
#include "wal_commit_observer.h"

namespace consensus {

void ReadyFlusher::Start() {
  auto f = [&]() {
    yaraft::Ready* rd;
    if (!readyQueue_.try_dequeue(rd)) {
      // wait 10ms if no ready in the queue
      usleep(1000 * 10);
    } else {
      flushReady(rd);
    }
  };
  FATAL_NOT_OK(worker_.StartLoop(f), "ReadyFlusher::Start");
}

void ReadyFlusher::Stop() {
  FATAL_NOT_OK(worker_.Stop(), "ReadyFlusher::Stop");
}

Status ReadyFlusher::flushReady(yaraft::Ready* rd) {
  if (rd->hardState) {
  }

  if (rd->entries) {
    RETURN_NOT_OK(wal_->AppendEntries(*rd->entries));
  }

  if (!rd->messages.empty()) {
    peers_->Pass(rd->messages);
    rd->messages.clear();
  }

  onReadyFlushed(rd);
  return Status::OK();
}

void ReadyFlusher::onReadyFlushed(yaraft::Ready* rd) {
  // committedIndex has changed
  if (rd->hardState && rd->hardState->has_commit()) {
    walCommitObserver_->Notify(rd->hardState->commit());
  }
}

}  // namespace consensus
