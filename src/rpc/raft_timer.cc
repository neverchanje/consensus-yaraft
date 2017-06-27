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

#include "rpc/raft_timer.h"
#include "rpc/raft_server.h"

#include <boost/bind.hpp>

namespace consensus {
namespace rpc {

// Timeout granularity of timer in milli-seconds.
static uint32_t kTimerGranularity = 10;

RaftTimer::RaftTimer(const RaftServiceImpl *server)
    : timer_(io_service_), io_strand_(io_service_), client_(server->Url()) {
  FMT_LOG(INFO, "Starting raft timer with timeout granularity: {}ms", kTimerGranularity);

  timer_.expires_from_now(boost::posix_time::milliseconds(kTimerGranularity));
  timer_.async_wait(io_strand_.wrap(boost::bind(&RaftTimer::onTimeout, this)));
}

void RaftTimer::Run() {
  io_service_.run();
}

void RaftTimer::Stop() {
  io_service_.stop();
}

void RaftTimer::onTimeout() {
  auto sw = client_.Tick(kTimerGranularity);
  if (!sw.IsOK()) {
    LOG(ERROR) << "RaftTimer::onTimeout: " << sw.GetStatus();
    // retry if error (nevertheless it's not supposed to have any error)
  }

  timer_.expires_from_now(boost::posix_time::milliseconds(kTimerGranularity));
  timer_.async_wait(io_strand_.wrap(boost::bind(&RaftTimer::onTimeout, this)));
}

}  // namespace rpc
}  // namespace consensus
