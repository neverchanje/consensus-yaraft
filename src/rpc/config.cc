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

#include "rpc/config.h"
#include "base/logging.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace consensus {
namespace rpc {

DEFINE_string(initial_cluster, "", "initial membership of the cluster");
DEFINE_string(name, "", "");

#define ERROR_IF_NOT(cond)                                           \
  do {                                                               \
    if (!(cond))                                                     \
      return Status::Make(Error::BadConfig, "Check failed: " #cond); \
  } while (0)

Status ParseClusterMembershipFromGFlags(std::map<std::string, std::string> *peerMap) {
  using namespace silly;

  std::vector<std::string> servers;
  boost::split(servers, FLAGS_initial_cluster, [](char c) -> bool { return c == ';'; });

  for (std::string &server : servers) {
    boost::trim(server);
    if (server.empty()) {
      continue;
    }

    auto sep = std::find(server.begin(), server.end(), '=');
    ERROR_IF_NOT(sep != server.end());
    ERROR_IF_NOT(sep != server.begin());
    ERROR_IF_NOT(sep != std::prev(server.end()));
    size_t sepIndex = std::distance(server.begin(), sep);

    std::string serverName = server.substr(0, sepIndex);
    boost::trim(serverName);

    std::string serverAddress = server.substr(sepIndex + 1, server.length() - sepIndex);
    peerMap->insert(std::make_pair(std::move(serverName), std::move(serverAddress)));
  }
  return Status::OK();
}

}  // namespace rpc
}  // namespace consensus
