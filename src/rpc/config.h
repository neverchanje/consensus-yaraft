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

#include <map>

#include "peer.h"

#include <gflags/gflags.h>

#pragma once

namespace consensus {
namespace rpc {

DECLARE_string(name);

// --initial_cluster="<name>:<ip>, ..."
DECLARE_string(initial_cluster);
Status ParseClusterMembershipFromGFlags(std::map<std::string, std::string> *peerMap);

DECLARE_string(wal_dir);

}  // namespace rpc
}  // namespace consensus
