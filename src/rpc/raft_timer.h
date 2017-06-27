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

#include "raft_client.h"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/strand.hpp>

namespace consensus {
namespace rpc {

class RaftServiceImpl;
class RaftTimer {
 public:
  explicit RaftTimer(const RaftServiceImpl *server);

  void Run();

  void Stop();

 private:
  void onTimeout();

 private:
  boost::asio::io_service io_service_;
  boost::asio::strand io_strand_;
  boost::asio::deadline_timer timer_;

  SyncRaftClient client_;
};

}  // namespace rpc
}  // namespace consensus
