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
#include <yaraft/pb_utils.h>

namespace consensus {
namespace wal {

using yaraft::EntryVec;
using yaraft::PBEntry;
using yaraft::MemoryStorage;
using yaraft::pb::Entry;

inline bool operator==(const Entry& e1, const Entry& e2) {
  bool result = (e1.term() == e2.term()) && (e1.index() == e2.index());
  return result;
}

inline bool operator!=(Entry e1, Entry e2) {
  return !(e1 == e2);
}

inline bool operator==(EntryVec v1, EntryVec v2) {
  if (v1.size() != v2.size())
    return false;
  auto it1 = v1.begin();
  auto it2 = v2.begin();
  while (it1 != v1.end()) {
    if (*it1++ != *it2++)
      return false;
  }
  return true;
}

class LogManagerTest : public BaseTest {
 public:
  LogManagerTest() {}
};

TEST_F(LogManagerTest, AppendEntries) {
  using namespace yaraft;

  TestDirGuard g(CreateTestDirGuard());

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
}

TEST_F(LogManagerTest, AppendToMemStore) {
  struct TestData {
    EntryVec vec;

    Error::ErrorCodes code;
    EntryVec expected;
  } tests[] = {
      {{PBEntry().Index(2).Term(1).v, PBEntry().Index(3).Term(1).v},
       Error::OK,
       {PBEntry().Index(1).Term(1).v, PBEntry().Index(2).Term(1).v, PBEntry().Index(3).Term(1).v}},

      {{PBEntry().Index(2).Term(2).v, PBEntry().Index(3).Term(1).v}, Error::YARaftERR, {}},

      {{PBEntry().Index(2).Term(2).v, PBEntry().Index(2).Term(2).v},
       Error::OK,
       {PBEntry().Index(1).Term(1).v, PBEntry().Index(2).Term(2).v}},

      {{PBEntry().Index(2).Term(2).v, PBEntry().Index(3).Term(3).v, PBEntry().Index(2).Term(4).v},
       Error::OK,
       {PBEntry().Index(1).Term(1).v, PBEntry().Index(2).Term(4).v}},
  };

  for (auto t : tests) {
    MemoryStorage memstore;
    auto& vec = memstore.TEST_Entries();
    vec.clear();
    vec.push_back(PBEntry().Index(1).Term(1).v);

    auto s = AppendToMemStore(t.vec, &memstore);
    ASSERT_EQ(s.Code(), t.code);

    if (s.IsOK()) {
      ASSERT_TRUE(t.expected == memstore.TEST_Entries());
    }
  }
}

}  // namespace wal
}  // namespace consensus