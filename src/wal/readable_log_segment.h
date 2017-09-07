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

#pragma once

#include "base/status.h"
#include "wal/segment_meta.h"

#include <yaraft/memory_storage.h>

namespace consensus {
namespace wal {

extern Status ReadSegmentIntoMemoryStorage(const Slice &fname, yaraft::MemoryStorage *memstore,
                                           SegmentMetaData *metaData, bool verifyChecksum);

// ReadableLogSegment reads the data of a segment into memory all at once.
// It's sufficient because it's only used in wal recovery.
class ReadableLogSegment {
 public:
  ReadableLogSegment(const Slice &scratch, yaraft::MemoryStorage *memStore,
                     SegmentMetaData *metaData, bool verifyChecksum)
      : remain_(scratch.size()),
        buf_(scratch.data()),
        metaData_(metaData),
        memStore_(memStore),
        verifyChecksum_(verifyChecksum) {}

  Status ReadHeader();

  Status ReadRecord();

  bool Eof();

 private:
  Status checkRemain(size_t need);

  void advance(size_t size);

 private:
  const char *buf_;
  size_t remain_;
  yaraft::MemoryStorage *memStore_;
  SegmentMetaData *metaData_;

  const bool verifyChecksum_;
};

}  // namespace wal
}  // namespace consensus