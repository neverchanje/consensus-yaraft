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

#include "base/coding.h"
#include "base/env_util.h"
#include "base/status.h"
#include "wal/format.h"
#include "wal/options.h"
#include "wal/segment_meta.h"

#include <boost/crc.hpp>
#include <google/protobuf/io/coded_stream.h>
#include <silly/likely.h>
#include <yaraft/raftpb.pb.h>

namespace consensus {
namespace wal {

// ReadableLogSegment saves all data of a segment into memory.
class ReadableLogSegment {
 public:
  ~ReadableLogSegment() {
    delete[] buf_;
  }

  static StatusWith<ReadableLogSegment *> Create(const SegmentMetaData &meta) {
    auto bufLen = meta.fileSize;
    auto buf = new char[bufLen];
    memset(buf, static_cast<int>(bufLen), '\0');

    Slice s;
    RETURN_NOT_OK(env_util::ReadFullyToAllocatedBuffer(meta.fileName, &s, buf));
    return new ReadableLogSegment(buf, bufLen);
  }

  ReadableLogSegment(char *buf, size_t len) : buf_(buf), offset_(0), remain_(len) {}

  Status SkipHeader() {
    CHECK_EQ(offset_, 0);
    CHECK_GE(remain_, kSegmentHeaderSize);
    advance(kSegmentHeaderSize);
    return Status::OK();
  }

  StatusWith<yaraft::pb::Entry> ReadEntry() {
    CHECK_GE(remain_, 8);

    uint32_t crc32;
    crc32 = DecodeFixed32(buf_ + offset_);
    advance(4);

    uint32_t size;
    size = DecodeFixed32(buf_ + offset_);
    advance(4);

    CHECK_GE(remain_, size);
    std::string entryBuf(buf_ + offset_, size);
    advance(size);

    boost::crc_32_type crc;
    crc.process_bytes(entryBuf.data(), size);
    if (crc.checksum() != crc32) {
      return Status::Make(Error::Corruption, "");
    }

    yaraft::pb::Entry ent;
    ent.ParseFromString(entryBuf);

    DCHECK_EQ(ent.ByteSize(), size);
    return ent;
  }

  size_t Offset() const {
    return offset_;
  }

 private:
  void advance(size_t len) {
    offset_ += len;
    remain_ -= len;
  }

 private:
  char *buf_;
  size_t offset_;
  size_t remain_;
};

}  // namespace wal
}  // namespace consensus