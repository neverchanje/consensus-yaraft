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

#include <vector>

#include "base/status.h"

#include <yaraft/pb/raftpb.pb.h>

namespace consensus {
namespace rpc {

class Cluster {
 public:
  virtual ~Cluster() = default;

  virtual Status Pass(std::vector<yaraft::pb::Message>& mails) = 0;

  static Cluster* Default(const std::map<uint64_t, std::string>& initialCluster);
};

}  // namespace rpc
}  // namespace consensus
