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

#include "base/env.h"
#include "base/logging.h"
#include "wal/format.h"
#include "wal/log_manager.h"
#include "wal/segment_meta.h"

#include <fmt/format.h>
#include <silly/likely.h>

namespace consensus {
namespace wal {

class LogWriter {
 public:
  // Create a log writer for the new log segment.
  static StatusWith<LogWriter *> New(LogManager *manager) {
    uint64_t newSegId = manager->files_.size() + 1;
    uint64_t newSegStart = manager->lastIndex_ + 1;
    std::string fname = manager->options_.log_dir + "/" + SegmentFileName(newSegId, newSegStart);
    FMT_LOG(INFO, "creating new segment segId: {}, firstId: {}", newSegId, newSegStart);

    WritableFile *wf;
    ASSIGN_IF_OK(Env::Default()->NewWritableFile(fname, Env::CREATE_NON_EXISTING), wf);

    return new LogWriter(wf, fname, manager->options_.log_segment_size);
  }

  LogWriter(WritableFile *wf, const std::string &fname, size_t logSegmentSize)
      : file_(wf), empty_(true), logSegmentSize_(logSegmentSize) {
    meta_.fileName = fname;
  }

  // Append log entries in range [begin, end) & hard state into the underlying segment.
  // If the current write is beyond the configured segment size, it returns a
  // iterator points at the next entry to be appended.
  StatusWith<ConstPBEntriesIterator> Append(ConstPBEntriesIterator begin,
                                            ConstPBEntriesIterator end,
                                            const yaraft::pb::HardState *hs = nullptr);

  Status Sync() {
    return file_->Sync();
  }

  Status Finish(SegmentMetaData *meta) {
    RETURN_NOT_OK(file_->Sync());
    RETURN_NOT_OK(file_->Close());

    *meta = meta_;
    return Status::OK();
  }

 private:
  void saveHardState(const yaraft::pb::HardState &hs, char *dest, size_t *offset);

  void saveEntries(ConstPBEntriesIterator begin, ConstPBEntriesIterator end, char *dest,
                   size_t *offset);

 private:
  friend class LogWriterTest;

 private:
  std::unique_ptr<WritableFile> file_;
  SegmentMetaData meta_;

  const size_t logSegmentSize_;

  bool empty_;
};

}  // namespace wal
}  // namespace consensus