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
#include "base/slice.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace consensus {
namespace rpc {

DEFINE_string(initial_cluster, "", "initial membership of the cluster");

static Status parseUInt64(const Slice &str, uint64_t *val) {
  char *pEnd = nullptr;
  int errNum;

  errno = 0;
  (*val) = strtoull(str.data(), &pEnd, 10);
  errNum = errno;

  // val can never be 0.
  if (*val == 0 || (pEnd - str.data()) != str.Len())
    return Status::Make(Error::BadConfig, "NumberLong: Invalid conversion from string to integer");

  if (errNum == ERANGE)
    return Status::Make(Error::BadConfig, "NumberLong: Value cannot fit in uint64");

  return Status::OK();
}

#define ERROR_IF_NOT(cond)                                           \
  do {                                                               \
    if (!(cond))                                                     \
      return Status::Make(Error::BadConfig, "Check failed: " #cond); \
  } while (0)

#define ERROR_IF_NOT_EQ(a, b) ERROR_IF_NOT((a) == (b))

Status ParseClusterMembershipFromGFlags(std::map<uint64_t, std::string> *peerMap) {
  using namespace silly;

  std::vector<std::string> servers;
  boost::split(servers, FLAGS_initial_cluster, [](char c) { return c == ';'; });

  for (std::string &server : servers) {
    boost::trim(server);
    if (server.empty()) {
      continue;
    }

    auto sep = std::find(server.begin(), server.end(), '=');
    ERROR_IF_NOT(sep != server.end());
    ERROR_IF_NOT(sep != server.begin());
    size_t sepIndex = std::distance(server.begin(), sep);

    std::string serverName = server.substr(0, sepIndex);
    boost::trim(serverName);

    constexpr Slice prefix = "server."_sl;
    ERROR_IF_NOT(serverName.length() > prefix.Len());
    ERROR_IF_NOT_EQ(serverName.substr(0, prefix.Len()), prefix.data());

    Slice serverIdStr(serverName);
    serverIdStr.Skip(prefix.Len());
    uint64_t serverId;

    RETURN_NOT_OK(parseUInt64(serverIdStr, &serverId));

    std::string serverAddress = server.substr(sepIndex + 1, server.length() - sepIndex);
    peerMap->insert(std::make_pair(serverId, std::move(serverAddress)));
  }
  return Status::OK();
}

}  // namespace rpc
}  // namespace consensus
