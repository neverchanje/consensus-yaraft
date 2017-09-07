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

#include "wal/readable_log_segment.h"
#include "base/coding.h"
#include "base/env_util.h"
#include "base/logging.h"
#include "wal/format.h"

#include <boost/crc.hpp>

namespace consensus {
namespace wal {

Status ReadSegmentIntoMemoryStorage(const Slice &fname, yaraft::MemoryStorage *memStore,
                                    SegmentMetaData *metaData, bool verifyChecksum) {
  char *buf;
  Slice s;
  RETURN_NOT_OK(env_util::ReadFullyToBuffer(fname, &s, &buf));

  ReadableLogSegment seg(s, memStore, metaData, verifyChecksum);
  RETURN_NOT_OK_APPEND(seg.ReadHeader(), fmt::format(" [segment: {}] ", fname.ToString()));
  while (!seg.Eof()) {
    RETURN_NOT_OK_APPEND(seg.ReadRecord(), fmt::format(" [segment: {}] ", fname.ToString()));
  }

  return Status::OK();
}

Status ReadableLogSegment::ReadHeader() {
  // check magic
  RETURN_NOT_OK_APPEND(checkRemain(kLogSegmentHeaderMagic.size()), "[bad magic length]");
  Slice magic(buf_, kLogSegmentHeaderMagic.size());
  if (UNLIKELY(kLogSegmentHeaderMagic.Compare(magic) != 0)) {
    return FMT_Status(Corruption, "bad header magic: {}", magic.ToString());
  }
  advance(kLogSegmentHeaderMagic.size());

  return Status::OK();
}

Status ReadableLogSegment::ReadRecord() {
  RETURN_NOT_OK_APPEND(checkRemain(kLogBatchHeaderSize), " [bad batch header] ");

  uint32_t crc = DecodeFixed32(buf_);
  uint32_t len = DecodeFixed32(buf_ + 4);
  advance(kLogBatchHeaderSize);

  RETURN_NOT_OK_APPEND(checkRemain(len), " [bad batch length] ");

  if (verifyChecksum_) {
    boost::crc_32_type crc32;
    crc32.process_bytes(buf_, len);
    if (crc32.checksum() != crc) {
      return FMT_Status(Corruption, "bad checksum");
    }
  }

  Slice record(buf_, len);
  while (record.Len() > 0) {
    auto type = static_cast<RecordType>(record[0]);
    record.Skip(kRecordHeaderSize);

    Slice data;
    if (UNLIKELY(!GetLengthPrefixedSlice(&record, &data))) {
      return Status::Make(Error::Corruption, "bad record");
    }

    if (type == kLogEntryType) {
      yaraft::pb::Entry e;
      e.ParseFromArray(data.RawData(), data.Len());
      memStore_->Append(e);
      metaData_->numEntries++;
    } else if (type == kHardStateType) {
      yaraft::pb::HardState hs;
      hs.ParseFromArray(data.RawData(), data.Len());
      memStore_->SetHardState(hs);
    }
  }
  advance(len);

  return Status::OK();
}

bool ReadableLogSegment::Eof() {
  return remain_ == 0;
}

Status ReadableLogSegment::checkRemain(size_t need) {
  if (UNLIKELY(remain_ < need)) {
    return FMT_Status(Corruption, "segment is too small to contain {} number of bytes", need);
  }
  return Status::OK();
}

void ReadableLogSegment::advance(size_t size) {
  remain_ -= size;
  buf_ += size;
}

}  // namespace wal
}  // namespace consensus