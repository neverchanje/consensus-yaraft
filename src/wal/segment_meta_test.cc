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

#include "base/random.h"
#include "base/testing.h"
#include "wal/log_writer.h"
#include "wal/readable_log_segment.h"

using namespace consensus;
using namespace consensus::wal;
using namespace std;
using namespace yaraft;

class SegmentMetaTest : public BaseTest {
 public:
  SegmentMetaTest() : rng_(SeedRandom()) {}

  ~SegmentMetaTest() = default;

  // write a vector of random entries into wal
  ::consensus::StatusWith<SegmentMetaData> WriteTestSegment(const Slice& path,
                                                            const EntryVec& entries) {
    SegmentMetaData segMeta;
    segMeta.fileName = path.ToString();

    WritableFile* wf;
    ASSIGN_IF_OK(Env::Default()->NewWritableFile(path), wf);
    LogWriter writer(wf, segMeta);

    ConstPBEntriesIterator it;
    ASSIGN_IF_OK(writer.AppendEntries(entries.begin(), entries.end()), it);

    CHECK(it == entries.end());
    CHECK(!writer.Oversize());
    RETURN_NOT_OK(writer.Finish());

    return segMeta;
  }

  EntryVec RandomEntryVec() {
    EntryVec vec(kEntriesNumInSegment);

    for (size_t i = 1; i <= kEntriesNumInSegment; i++) {
      auto e = PBEntry()
                   .Term(1)
                   .Index(static_cast<uint64_t>(i))
                   .Data(RandomString(kEntryByteSize, &rng_))
                   .v;
      vec[i - 1] = e;
    }
    return vec;
  }

  constexpr static size_t kEntriesNumInSegment = 100;
  constexpr static size_t kEntryByteSize = 20;

 protected:
  Random rng_;
};

// This test verifies an encoded entry can be decoded correctly.
TEST_F(SegmentMetaTest, EncodeAndDecode) {
  for (int i = 0; i < 1000; i++) {
    auto expect = yaraft::PBEntry().Index(1).Term(2).Data(RandomString(20, &rng_)).v;

    size_t bufLen = kSegmentHeaderSize + kEntryHeaderSize + expect.ByteSize();
    SegmentMetaData meta;
    char* s = new char[bufLen];
    s[0] = 0;

    size_t tmp;
    EncodeToAllocatedArray(expect, s + kSegmentHeaderSize, &tmp);

    ReadableLogSegment seg(s, bufLen);
    yaraft::pb::Entry actual;
    {
      auto sw = seg.ReadEntry();
      ASSERT_OK(sw.GetStatus());
      actual = sw.GetValue();
    }
    ASSERT_EQ(expect.SerializeAsString(), actual.SerializeAsString());
  }
}