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

#include "wal/log_manager.h"
#include "base/testing.h"

#include <gtest/gtest.h>

namespace consensus {
namespace wal {

class LogManagerTest : public BaseTest {
 public:
  LogManagerTest() {}

  void AppendSegMeta(LogManager& m, SegmentMetaData& meta) {
    m.files_.push_back(std::move(meta));
  }
};

TEST_F(LogManagerTest, FindSegmentId) {
  LogManager manager;

  struct TestData {
    std::vector<std::pair<uint64_t, uint64_t>> ranges;

    Error::ErrorCodes code;
  } tests[] = {
      {{{1, 5}, {6, 10}}, Error::OK},
      {{{1, 3}, {4, 7}}, Error::OutOfBound},
      {{{9, 12}, {13, 16}}, Error::LogCompacted},
  };

  for (auto t : tests) {
    for (auto r : t.ranges) {
      SegmentMetaData meta;
      meta.range = r;
      AppendSegMeta(manager, meta);
    }

    auto sw = manager.FindSegmentId(8);

    if (!sw.IsOK()) {
      ASSERT_EQ(sw.GetStatus().Code(), t.code);
    } else {
      ASSERT_EQ(sw.GetValue(), 1);
    }
  }
}

}  // namespace wal
}  // namespace consensus