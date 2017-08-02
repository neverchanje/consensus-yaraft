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
#include "base/coding.h"

#include <boost/crc.hpp>

namespace consensus {
namespace wal {

StatusWith<ConstPBEntriesIterator> LogWriter::Append(ConstPBEntriesIterator begin,
                                                     ConstPBEntriesIterator end,
                                                     const yaraft::pb::HardState *hs) {
  if (empty_) {
    RETURN_NOT_OK(file_->Append(kLogSegmentHeaderMagic));
    empty_ = false;
  }

  ssize_t remains = FLAGS_log_segment_size - file_->Size();
  size_t totalSize = kLogBatchHeaderSize;

  if (hs) {
    totalSize += kRecordHeaderSize + hs->ByteSize();
  }

  bool writeEntries = false;
  auto newBegin = begin;
  if (totalSize < remains && begin != end) {
    writeEntries = true;
    for (; newBegin != end; newBegin++) {
      if (totalSize < remains) {
        totalSize += kRecordHeaderSize + VarintLength(newBegin->ByteSize()) + newBegin->ByteSize();
      } else {
        break;
      }
    }
  }

  std::string scratch(totalSize, '\0');
  size_t offset = kLogBatchHeaderSize;

  if (hs) {
    saveHardState(*hs, &scratch[offset], &offset);
  }

  if (writeEntries) {
    saveEntries(begin, newBegin, &scratch[offset], &offset);
  }

  size_t dataLen = totalSize - kLogBatchHeaderSize;

  // len field
  EncodeFixed32(&scratch[4], static_cast<uint32_t>(dataLen));

  // crc field
  boost::crc_32_type crc;
  crc.process_bytes(&scratch[kLogBatchHeaderSize], dataLen);
  EncodeFixed32(&scratch[0], static_cast<uint32_t>(crc.checksum()));

  RETURN_NOT_OK(file_->Append(scratch));

  meta_.numEntries += std::distance(begin, newBegin);
  return newBegin;
}

void LogWriter::saveHardState(const yaraft::pb::HardState &hs, char *dest, size_t *offset) {
  char *p = dest;
  p[0] = static_cast<char>(kHardStateType);

  p = EncodeVarint32(p + 1, hs.ByteSize());
  hs.SerializeToArray(p, hs.ByteSize());

  (*offset) += p - dest + hs.ByteSize();
}

void LogWriter::saveEntries(ConstPBEntriesIterator begin, ConstPBEntriesIterator end, char *dest,
                            size_t *offset) {
  char *p = dest;
  char type = static_cast<char>(kLogEntryType);

  for (auto it = begin; it != end; it++) {
    p[0] = type;
    p = EncodeVarint32(p + 1, it->ByteSize());
    it->SerializeToArray(p, it->ByteSize());
    p += it->ByteSize();
  }
  (*offset) += p - dest;
}

}  // namespace wal
}  // namespace consensus