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
#include "base/status.h"

#include <gtest/gtest.h>

using namespace consensus::rpc;
using namespace consensus;

TEST(Config, Parse) {
  struct TestData {
    std::string cmdParam;

    std::vector<std::pair<std::string, std::string>> expected;
    Error::ErrorCodes code;
  } tests[] = {
      {"server1=192.168.0.1:1080", {{"server1", "192.168.0.1:1080"}}, Error::OK},

      {"server1=192.168.0.1:1081;server2=192.168.0.2:1082;server3=192.168.0.3:1083;",
       {{"server1", "192.168.0.1:1081"},
        {"server2", "192.168.0.2:1082"},
        {"server3", "192.168.0.3:1083"}},
       Error::OK},

      // empty param
      {";;;", {}, Error::OK},

      {"server1 192.168.0.1:1080;", {}, Error::BadConfig},
  };

  for (auto& t : tests) {
    FLAGS_initial_cluster = t.cmdParam;

    std::map<std::string, std::string> peerMap;

    auto s = ParseClusterMembershipFromGFlags(&peerMap);
    ASSERT_EQ(s.Code(), t.code);

    ASSERT_EQ(peerMap.size(), t.expected.size());

    auto it = peerMap.begin();
    for (auto& p : t.expected) {
      ASSERT_EQ(p.first, it->first);
      ASSERT_EQ(p.second, it->second);

      it++;
    }
  }
}