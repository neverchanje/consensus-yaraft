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

std::string EncodeToString(const yaraft::pb::Entry &entry) {
  std::string buf(entry.ByteSize() + 8, '\0');
  char *p = &buf[0];

  size_t bufLen;
  EncodeToAllocatedArray(entry, p, &bufLen);
  CHECK_EQ(bufLen, buf.size());

  return buf;
}

void EncodeToArray(const yaraft::pb::Entry &entry, char **result, size_t *len) {
  (*result) = new char[entry.ByteSize() + 8];
  EncodeToAllocatedArray(entry, *result, len);
}

void EncodeToAllocatedArray(const yaraft::pb::Entry &entry, char *result, size_t *len) {
  char *p = result;
  entry.SerializeToArray(p + 8, entry.ByteSize());

  // 4-byte crc checksum
  boost::crc_32_type crc;
  crc.process_bytes(p + 8, static_cast<size_t>(entry.ByteSize()));
  EncodeFixed32(p, static_cast<uint32_t>(crc.checksum()));

  // 4-byte length
  EncodeFixed32(p + 4, static_cast<uint32_t>(entry.ByteSize()));

  (*len) = static_cast<size_t>(entry.ByteSize() + 8);
}

}  // namespace wal
}  // namespace consensus