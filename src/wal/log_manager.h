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
#include "wal/wal.h"

#include <yaraft/memory_storage.h>

namespace consensus {

class WritableFile;

namespace wal {

class LogWriter;

class LogManager;
using LogManagerUPtr = std::unique_ptr<LogManager>;

// Not-Thread-Safe
class LogManager : public WriteAheadLog {
 public:
  explicit LogManager(const WriteAheadLogOptions& options);

  // Recover from existing wal files.
  // The options.log_dir will be created when it's not existed.
  // All of the uncompacted log entries will be read into `memstore`.
  //
  // ASSERT: *memstore == null
  static Status Recover(const WriteAheadLogOptions& options, yaraft::MemStoreUptr* memstore,
                        LogManagerUPtr* pLogManager);

  ~LogManager() override;

  // Required: no holes between logs and msg.entries.
  Status Write(const PBEntryVec& vec, const yaraft::pb::HardState* hs) override;

  // naive implementation: delete all committed segments.
  Status GC(WriteAheadLog::CompactionHint* hint) override;

  Status Sync() override;

  Status Close() override;

  // the number of log segments
  size_t SegmentNum() const {
    return files_.size() + static_cast<size_t>(bool(current_));
  }

 private:
  Status doWrite(ConstPBEntriesIterator begin, ConstPBEntriesIterator end,
                 const yaraft::pb::HardState* hs);

  void finishCurrentWriter();

 private:
  friend class LogManagerTest;
  friend class LogWriter;

  // current_ always writes to the last log segment when it's not null.
  std::unique_ptr<LogWriter> current_;

  // metadata of the immutable segments
  // new segment will be appended when the current writer finishes
  std::vector<SegmentMetaData> files_;

  uint64_t lastIndex_;
  bool empty_;

  const WriteAheadLogOptions options_;
};

Status AppendToMemStore(yaraft::pb::Entry& e, yaraft::MemoryStorage* memstore);

inline Status AppendToMemStore(yaraft::EntryVec& vec, yaraft::MemoryStorage* memstore) {
  for (auto& e : vec) {
    auto s = AppendToMemStore(e, memstore);
    if (!s.IsOK())
      return s;
  }
  return Status::OK();
}

}  // namespace wal
}  // namespace consensus