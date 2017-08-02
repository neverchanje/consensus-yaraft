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

#include <cstddef>

namespace consensus {
namespace wal {

//  Format of the encoded log batch:
//
//  LogBatch := LogHeader Record+
//  LogHeader := Crc32 Length
//  Record := Type VarString
//
//  Crc32     -> 4 bytes, checksum of fields followed in the log block
//  Type      -> 1 byte, RecordType
//  VarString -> varint32 + bytes, encoded log entry or encoded hard state
//
//  Each segment composes of a series of log entries:
//
//  Segment := SegmentHeader LogBlock* SegmentFooter
//  SegmentHeader := Magic
//  SegmentFooter :=
//

constexpr static size_t kLogBatchHeaderSize = 4 + 4;
constexpr static size_t kRecordHeaderSize = 1;

enum RecordType {
  kHardStateType = 1,
  kLogEntryType = 2,
};

}  // namespace wal
}  // namespace consensus