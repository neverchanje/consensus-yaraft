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

#include "wal/segment_meta.h"
#include "base/random.h"
#include "base/testing.h"
#include "wal/log_writer.h"
#include "wal/readable_log_segment.h"

#include <yaraft/fluent_pb.h>

using namespace consensus;
using namespace consensus::wal;
using namespace std;
using namespace yaraft;

class SegmentMetaTest : public BaseTest {
 public:
  SegmentMetaTest() : rng_(SeedRandom()) {}

  ~SegmentMetaTest() = default;

 protected:
  Random rng_;
};

// This test verifies an encoded entry can be decoded correctly.
TEST_F(SegmentMetaTest, EncodeAndDecode) {
  for (int i = 0; i < 1000; i++) {
    auto expect = yaraft::PBEntry().Index(1).Term(2).Data(RandomString(20, &rng_)).v;
    SegmentMetaData meta;

    char* s = new char[kSegmentHeaderSize + kEntryHeaderSize + expect.ByteSize()];
    s[0] = '\0';
    EncodedToArray(expect, &s, &meta.fileSize);

    ReadableLogSegment seg(&meta, s);
    yaraft::pb::Entry actual;
    {
      auto sw = seg.ReadEntry();
      ASSERT_OK(sw.GetStatus());
      actual = sw.GetValue();
    }
    ASSERT_EQ(expect.SerializeAsString(), actual.SerializeAsString());
  }
}