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

#include <atomic>
#include <thread>

#include "background_worker.h"

namespace consensus {

class BackgroundWorker::Impl {
 public:
  explicit Impl() : stopped_(true) {}

  Status StartLoop(std::function<void()> recurringTask) {
    if (bgWorkerThread_.joinable()) {
      return Status::Make(Error::RuntimeError,
                          "BackgroundWorker::StartLoop: start an already started thread");
    }

    if (!recurringTask) {
      return Status::Make(Error::InvalidArgument,
                          "BackgroundWorker::StartLoop: the given task has no target");
    }

    stopped_.store(false);
    bgWorkerThread_ = std::thread([=]() {
      while (!stopped_.load()) {
        recurringTask();
      }
    });
    return Status::OK();
  }

  Status Stop() {
    if (!bgWorkerThread_.joinable()) {
      return Status::Make(Error::RuntimeError,
                          "BackgroundWorker::Stop: stop a not-yet-started thread");
    }

    stopped_.store(true);
    bgWorkerThread_.join();
    return Status::OK();
  }

  bool Stopped() const {
    return stopped_.load();
  }

 private:
  std::thread bgWorkerThread_;
  std::atomic_bool stopped_;
};

BackgroundWorker::BackgroundWorker() : impl_(new Impl) {}

Status BackgroundWorker::StartLoop(std::function<void()> recurringTask) {
  return impl_->StartLoop(recurringTask);
}

Status BackgroundWorker::Stop() {
  return impl_->Stop();
}

bool BackgroundWorker::Stopped() const {
  return impl_->Stopped();
}

BackgroundWorker::~BackgroundWorker() {}

}  // namespace consensus