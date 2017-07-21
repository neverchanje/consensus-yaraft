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

#include "base/background_worker.h"
#include "base/status.h"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/strand.hpp>

namespace consensus {

// RaftTimer is the background timer that ticks for every 1ms.
// In our implementation, it doesn't generate a task every 1ms, instead,
// it request RaftTaskExecutor to run RawNode::Tick 100 times for every 100ms.

class RaftTaskExecutor;
class RaftTimer {
 public:
  explicit RaftTimer(RaftTaskExecutor* executor);

  void Start();

  void Stop();

 private:
  RaftTaskExecutor* executor_;

  boost::asio::io_service io_service_;
  boost::asio::deadline_timer timer_;

  BackgroundWorker worker_;
};

}  // namespace consensus
