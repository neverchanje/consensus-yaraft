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

#include "base/coding.h"
#include "base/mock_env.h"
#include "base/random.h"
#include "base/testing.h"
#include "wal/format.h"
#include "wal/log_writer.h"
#include "wal/readable_log_segment.h"

namespace consensus {
namespace wal {

using namespace yaraft;

class LogWriterTest : public BaseTest {
 public:
  LogWriterTest() = default;

  // the segment can hold exactly `entriesInSegment` number of entries.
  EntryVec InitLogSegment(size_t entriesInSegment) {
    FLAGS_log_segment_size = kLogSegmentHeaderMagic.size() + kLogBatchHeaderSize;
    EntryVec vec;
    for (uint64_t i = 1; i <= entriesInSegment; i++) {
      vec.push_back(PBEntry().Index(i).Term(i).v);
      FLAGS_log_segment_size +=
          kRecordHeaderSize + VarintLength(vec.back().ByteSize()) + vec.back().ByteSize();
    }
    return vec;
  }

  void TestAppendEntries(size_t entriesInSegment) {
    auto vec = InitLogSegment(entriesInSegment);

    // write one more entry than the segment can hold
    vec.push_back(PBEntry().Index(entriesInSegment + 1).Term(entriesInSegment + 1).v);

    auto wf = new MockWritableFile;
    LogWriter writer(wf, "test-seg");
    ConstPBEntriesIterator it;
    ASSIGN_IF_ASSERT_OK(writer.Append(vec.begin(), vec.end()), it);

    ASSERT_EQ(std::distance(vec.cbegin(), it), entriesInSegment);
    ASSERT_EQ(wf->Data().size(), FLAGS_log_segment_size);

    SegmentMetaData metaData;
    ASSIGN_IF_ASSERT_OK(writer.Finish(), metaData);
    ASSERT_EQ(metaData.numEntries, entriesInSegment);
  }

  void TestEncodeAndDecode(size_t entriesInSegment) {
    auto expectedEntries = InitLogSegment(entriesInSegment);
    FLAGS_verifiy_checksums = true;

    auto wf = new MockWritableFile;
    LogWriter writer(wf, "test-seg");
    ASSERT_OK(writer.Append(expectedEntries.begin(), expectedEntries.end()));

    std::string fileData = wf->Data();
    ASSERT_EQ(fileData.size(), FLAGS_log_segment_size);

    SegmentMetaData meta;
    yaraft::MemoryStorage memStore;
    ReadableLogSegment seg(fileData, &memStore, &meta);

    // read from segment
    ASSERT_OK(seg.ReadHeader());
    while (!seg.Eof()) {
      ASSERT_OK(seg.ReadRecord());
    }

    EntryVec actualEntries(memStore.TEST_Entries().begin() + 1, memStore.TEST_Entries().end());
    ASSERT_EQ(actualEntries.size(), expectedEntries.size());
    for (int i = 0; i < actualEntries.size(); i++) {
      ASSERT_EQ(actualEntries[i].DebugString(), expectedEntries[i].DebugString());
    }
  }

 private:
};

// LogWriterTest.AppendEntries ensures that LogWriter::AppendEntries stops writing
// when entries is out of bound.
TEST_F(LogWriterTest, AppendEntries) {
  TestAppendEntries(10);
  TestAppendEntries(50);
  TestAppendEntries(100);
  TestAppendEntries(200);
  TestAppendEntries(500);
  TestAppendEntries(1000);
}

// This test verifies that data being written can be decoded correctly.
TEST_F(LogWriterTest, EncodeAndDecode) {
  TestEncodeAndDecode(10);
  TestEncodeAndDecode(200);
  TestEncodeAndDecode(500);
  TestEncodeAndDecode(1000);
  TestEncodeAndDecode(2000);
  TestEncodeAndDecode(10000);
}

}  // namespace wal
}  // namespace consensus