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

#include "base/task_queue.h"
#include "base/background_worker.h"
#include "base/logging.h"
#include "concurrentqueue/blockingconcurrentqueue.h"

namespace consensus {

class TaskQueue::Impl {
  using Runnable = std::function<void()>;

 public:
  void Enqueue(Runnable task) {
    queue_.enqueue(task);
  }

  void Start() {
    FATAL_NOT_OK(worker_.StartLoop([&]() {
      Runnable task;
      if (queue_.wait_dequeue_timed(task, std::chrono::milliseconds(50))) {
        task();
      }
    }),
                 "TaskQueue::Start");
  }

  void Stop() {
    FATAL_NOT_OK(worker_.Stop(), "TaskQueue::Stop");
  }

 private:
  moodycamel::BlockingConcurrentQueue<Runnable> queue_;
  BackgroundWorker worker_;
};

void TaskQueue::Enqueue(std::function<void()> task) {
  impl_->Enqueue(task);
}

TaskQueue::TaskQueue() : impl_(new Impl()) {
  impl_->Start();
}

TaskQueue::~TaskQueue() {
  impl_->Stop();
}

}  // namespace consensus