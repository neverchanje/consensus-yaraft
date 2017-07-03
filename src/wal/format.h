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

namespace consensus {
namespace wal {

//  Format of the encoded log entry:
//
//  LogEntry  := LogHeader Data
//  LogHeader := Crc32 Length
//
//  Crc32  -> 4 bytes, checksum of Data field
//  Length -> 4 bytes, length of data
//  Data   -> bytes,   byte-encoded yaraft.pb.Entry
//
//  Each segment composes of a series of log entries:
//
//  Segment := LogEntry*

constexpr static size_t kEntryHeaderSize = 8;

}  // namespace wal
}  // namespace consensus