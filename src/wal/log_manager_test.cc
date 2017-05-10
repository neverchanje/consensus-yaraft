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
#include "wal/format.h"
#include "wal/log_writer.h"
#include "wal/options.h"
#include "wal/segment_meta.h"

#include <gtest/gtest.h>
#include <yaraft/fluent_pb.h>

namespace consensus {
namespace wal {

class LogManagerTest : public BaseTest {
 public:
  LogManagerTest() {}
};

TEST_F(LogManagerTest, AppendEntries) {
  using namespace yaraft;

  ASSERT_OK(Env::Default()->CreateDirIfMissing(GetTestDir()));
  LogManager manager(GetTestDir());

  size_t totalEntries = 100;
  size_t entriesPerSegment = 10;

  size_t emptyEntrySize = PBEntry().Index(1).Term(1).v.ByteSize();
  FLAGS_log_segment_size = (emptyEntrySize + kEntryHeaderSize) * entriesPerSegment;

  EntryVec vec;
  for (int i = 1; i <= totalEntries; i++) {
    vec.push_back(PBEntry().Index(i).Term(i).v);
  }

  manager.AppendEntries(PBMessage().Entries(vec).v);
  ASSERT_EQ(manager.SegmentNum(), totalEntries / entriesPerSegment);

  ASSERT_OK(Env::Default()->DeleteRecursively(GetParentDir()));
}

}  // namespace wal
}  // namespace consensus