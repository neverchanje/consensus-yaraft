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

#include "wal/log_writer.h"
#include "base/mock_env.h"
#include "base/testing.h"
#include "wal/format.h"
#include "wal/readable_log_segment.h"
#include "wal/segment_meta.h"

#include <gtest/gtest.h>
#include <yaraft/fluent_pb.h>

namespace consensus {
namespace wal {

class LogWriterTest : public BaseTest {
 public:
  LogWriterTest() = default;

  LogWriter ConstructLogWriter(WritableFile* wf, SegmentMetaData meta) {
    return LogWriter(wf, meta);
  }
};

// LogWriterTest.AppendEntries ensures that LogWriter::AppendEntries stops writing
// when entries is out of bound.
TEST_F(LogWriterTest, AppendEntries) {
  using namespace yaraft;

  size_t totalEntries = 11;
  size_t entriesPerSegment = 10;

  size_t emptyEntrySize = PBEntry().Index(1).Term(1).v.ByteSize();
  FLAGS_log_segment_size = (emptyEntrySize + kEntryHeaderSize) * entriesPerSegment;

  EntryVec vec;
  for (int i = 1; i <= totalEntries; i++) {
    vec.push_back(PBEntry().Index(i).Term(i).v);
  }
  auto msg = PBMessage().Entries(vec).v;

  SegmentMetaData meta;
  auto wf = new MockWritableFile;
  auto writer = ConstructLogWriter(wf, meta);
  ConstPBEntriesIterator it;
  {
    auto sw = writer.AppendEntries(msg.entries().begin(), msg.entries().end());
    ASSERT_OK(sw);
    it = sw.GetValue();
  }

  ASSERT_EQ(std::distance(msg.entries().begin(), it), entriesPerSegment);
}

// This test verifies that pb being encoded can be correctly decoded.
TEST_F(LogWriterTest, EncodeAndDecode) {
  using namespace yaraft;

  auto e = PBEntry().Index(1).Term(1).Data("some data").v;
  {
    char* s;
    size_t len;
    EncodeToArray(e, &s, &len);

    ReadableLogSegment reader(s, len);

    auto sw = reader.ReadEntry();
    ASSERT_OK(sw);
    ASSERT_EQ(sw.GetValue().SerializeAsString(), e.SerializeAsString());
  }

  {
    size_t len = e.ByteSize() + kEntryHeaderSize;
    char* s = new char[len];
    EncodeToAllocatedArray(e, s, &len);

    ReadableLogSegment reader(s, len);

    auto sw = reader.ReadEntry();
    ASSERT_OK(sw);
    ASSERT_EQ(sw.GetValue().SerializeAsString(), e.SerializeAsString());
  }
}

}  // namespace wal
}  // namespace consensus