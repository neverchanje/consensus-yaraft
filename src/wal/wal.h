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

#include <yaraft/memory_storage.h>
#include <yaraft/pb/raftpb.pb.h>

namespace consensus {
namespace wal {

using PBEntryVec = std::vector<yaraft::pb::Entry>;
using PBEntriesIterator = PBEntryVec::iterator;
using ConstPBEntriesIterator = PBEntryVec::const_iterator;

class WriteAheadLog {
 public:
  virtual ~WriteAheadLog() = default;

  // Append log entries into underlying storage.
  virtual Status AppendEntries(const PBEntryVec& entries) = 0;

  struct CompactionHint {};

  // Abandon the unused logs.
  virtual Status GC(CompactionHint* hint) = 0;

  // Default implementation of WAL.
  static Status Default(const std::string& logsDir, WriteAheadLog** wal,
                        yaraft::MemoryStorage* memstore);
};

}  // namespace wal
}  // namespace consensus