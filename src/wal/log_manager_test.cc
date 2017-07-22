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

#include "base/testing.h"
#include "wal/log_manager.h"
#include "wal/log_writer.h"
#include "wal/readable_log_segment.h"

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

TEST_F(LogManagerTest, AppendToOneSegment) {
  using namespace yaraft;

  struct TestData {
    size_t totalEntries;
  } tests[] = {{10}, {50}, {100}, {1000}};

  for (auto t : tests) {
    TestDirGuard g(CreateTestDirGuard());

    LogManager manager(GetTestDir());

    // Always writes to only one segment
    FLAGS_log_segment_size = 1024 * 1024 * 64;  // sufficient enough to hold all logs in one segment

    EntryVec vec;
    for (uint64_t i = 1; i <= t.totalEntries; i++) {
      vec.push_back(PBEntry().Index(i).Term(i).v);
    }
    ASSERT_OK(manager.AppendEntries(vec));

    // flush data into file
    ASSERT_OK(manager.Close());

    ReadableLogSegment* seg;
    ASSIGN_IF_ASSERT_OK(ReadableLogSegment::Create(GetTestDir() + "/" + SegmentFileName(1, 1)),
                        seg);
    std::unique_ptr<ReadableLogSegment> d(seg);

    for (int i = 0; i < t.totalEntries; i++) {
      auto sw = seg->ReadEntry();
      ASSERT_OK(sw);
      ASSERT_TRUE(sw.GetValue() == vec[i]);
    }
  }
}

TEST_F(LogManagerTest, AppendToMemStore) {
  using E = PBEntry;

  struct TestData {
    EntryVec vec;

    Error::ErrorCodes code;
    EntryVec expected;
  } tests[] = {
      {{E().Index(2).Term(1).v, E().Index(3).Term(1).v},
       Error::OK,
       {E().Index(2).Term(1).v, E().Index(3).Term(1).v}},

      {{E().Index(2).Term(2).v, E().Index(3).Term(1).v}, Error::YARaftError, {}},

      {{E().Index(2).Term(2).v, E().Index(2).Term(2).v}, Error::OK, {E().Index(2).Term(2).v}},

      {{E().Index(2).Term(2).v, E().Index(3).Term(3).v, E().Index(2).Term(4).v},
       Error::OK,
       {E().Index(2).Term(4).v}},

      {{E().Index(2).Term(1).v, E().Index(3).Term(1).v, E().Index(4).Term(2).v,
        E().Index(5).Term(3).v, E().Index(6).Term(3).v, E().Index(7).Term(3).v,
        E().Index(5).Term(4).v},
       Error::OK,
       {E().Index(2).Term(1).v, E().Index(3).Term(1).v, E().Index(4).Term(2).v,
        E().Index(5).Term(4).v}},
  };

  for (auto t : tests) {
    MemoryStorage memstore;
    memstore.TEST_Entries().clear();
    memstore.TEST_Entries().emplace_back(E().Index(1).Term(1).v);

    auto s = AppendToMemStore(t.vec, &memstore);
    ASSERT_EQ(s.Code(), t.code);

    if (s.IsOK()) {
      EntryVec& actual = memstore.TEST_Entries();
      std::move(std::next(actual.begin()), actual.end(), actual.begin());
      actual.pop_back();

      ASSERT_TRUE(t.expected == memstore.TEST_Entries());
    }
  }
}

// This test verifies that no logs will be loaded when LogManager recovers from empty directory.
TEST_F(LogManagerTest, RecoverFromEmtpyDirectory) {
  TestDirGuard g(CreateTestDirGuard());
  yaraft::MemoryStorage memstore;
  std::unique_ptr<LogManager> m;
  {
    auto sw = LogManager::Recover(GetTestDir(), &memstore);
    ASSERT_OK(sw);
    m.reset(sw.GetValue());
  }

  ASSERT_EQ(memstore.TEST_Entries().size(), 1);
}

TEST_F(LogManagerTest, Recover) {
  struct TestData {
    int logsNum;
  } tests[] = {
      {10}, {100}, {1000},
  };

  for (auto t : tests) {
    TestDirGuard g(CreateTestDirGuard());

    FLAGS_log_segment_size = 1024;

    EntryVec expected;
    for (uint64_t i = 1; i <= t.logsNum; i++) {
      expected.push_back(PBEntry().Index(i).Term(i).v);
    }

    size_t segNum;
    {
      LogManager m(GetTestDir());
      ASSERT_OK(m.AppendEntries(expected));
      segNum = m.SegmentNum();
      ASSERT_OK(m.Close());
    }

    yaraft::MemoryStorage memstore;
    LogManager* m;
    ASSIGN_IF_ASSERT_OK(LogManager::Recover(GetTestDir(), &memstore), m);
    std::unique_ptr<LogManager> d(m);

    ASSERT_EQ(segNum, m->SegmentNum());

    auto& actual = memstore.TEST_Entries();
    for (int i = 1; i < actual.size(); i++) {
      ASSERT_TRUE(expected[i - 1] == actual[i]);
    }
  }
}

}  // namespace wal
}  // namespace consensus