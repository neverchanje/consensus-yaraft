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

#include <future>

#include "raft_task_executor.h"
#include "raft_timer.h"

#include "base/background_worker.h"
#include "base/logging.h"
#include "base/simple_channel.h"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/strand.hpp>

namespace consensus {

// Timeout granularity of timer in milli-seconds.
static uint32_t kTimerGranularity = 100;

class RaftTimer::Impl {
 public:
  Impl() : timer_(io_service_) {
    FMT_LOG(INFO, "Set up raft timer with timeout granularity: {}ms", kTimerGranularity);
  }

  void Stop() {
    FATAL_NOT_OK(worker_.Stop(), "RaftTimer::Stop");

    io_service_.stop();
  }

  void Start() {
    io_service_.run();

    worker_.StartLoop([&]() {
      timer_.expires_from_now(boost::posix_time::milliseconds(kTimerGranularity));
      timer_.wait();

      std::lock_guard<std::mutex> g(mu_);
      for (auto& executor : executors_) {
        Barrier channel;
        executor->Submit([&](yaraft::RawNode* node) {
          for (uint32_t i = 0; i < kTimerGranularity; i++) {
            node->Tick();
          }
          channel.Signal();
        });
        channel.Wait();
      }
    });
  }

  void Register(RaftTaskExecutor* executor) {
    std::lock_guard<std::mutex> g(mu_);
    executors_.push_back(executor);
  }

 private:
  std::vector<RaftTaskExecutor*> executors_;
  std::mutex mu_;

  boost::asio::io_service io_service_;
  boost::asio::deadline_timer timer_;

  BackgroundWorker worker_;
};

RaftTimer::RaftTimer() : impl_(new Impl) {
  impl_->Start();
}

RaftTimer::~RaftTimer() {
  impl_->Stop();
}

void RaftTimer::Register(RaftTaskExecutor* executor) {
  impl_->Register(executor);
}

}  // namespace consensus
