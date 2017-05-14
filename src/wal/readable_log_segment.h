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
#include <fmt/format.h>
#include <google/protobuf/io/coded_stream.h>
#include <silly/likely.h>
#include <yaraft/raftpb.pb.h>

namespace consensus {
namespace wal {

// ReadableLogSegment saves all data of a segment into memory.
class ReadableLogSegment {
 public:
  struct Header {
    bool committed;
  };

 public:
  ~ReadableLogSegment() {
    delete[] buf_;
  }

  static StatusWith<ReadableLogSegment *> Create(const Slice &fname) {
    char *buf;
    Slice s;
    RETURN_NOT_OK(env_util::ReadFullyToBuffer(fname, &s, &buf));
    return new ReadableLogSegment(buf, s.Len());
  }

  ReadableLogSegment(char *buf, size_t len) : buf_(buf), offset_(0), remain_(len) {}

  StatusWith<Header> ReadHeader() {
    CHECK_EQ(offset_, 0);
    RETURN_NOT_OK(checkEnough(kSegmentHeaderSize));

    Header header;
    header.committed = static_cast<bool>(buf_[0]);
    advance(kSegmentHeaderSize);
    return header;
  }

  StatusWith<yaraft::pb::Entry> ReadEntry() {
    RETURN_NOT_OK(checkEnough(8));

    uint32_t crc32;
    crc32 = DecodeFixed32(buf_ + offset_);
    advance(4);

    uint32_t size;
    size = DecodeFixed32(buf_ + offset_);
    advance(4);

    RETURN_NOT_OK(checkEnough(size));
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

  size_t Remain() const {
    return remain_;
  }

  bool Eof() const {
    return remain_ == 0;
  }

 private:
  void advance(size_t len) {
    offset_ += len;
    remain_ -= len;
  }

  inline Status checkEnough(size_t need) const {
    if (remain_ < need) {
      return Status::Make(Error::Corruption, fmt::format("remain_({}) < {}", remain_, need));
    }
    return Status::OK();
  }

 private:
  char *buf_;
  size_t offset_;
  size_t remain_;
};

}  // namespace wal
}  // namespace consensus