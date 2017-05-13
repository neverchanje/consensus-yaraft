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
#include "wal/log_manager.h"
#include "wal/options.h"
#include "wal/segment_meta.h"
#include "wal/wal.h"

#include <fmt/format.h>
#include <silly/likely.h>

namespace consensus {
namespace wal {

std::string EncodeToString(const yaraft::pb::Entry &entry);
void EncodeToArray(const yaraft::pb::Entry &entry, char **result, size_t *len);
void EncodeToAllocatedArray(const yaraft::pb::Entry &entry, char *result, size_t *len);

class LogWriter {
 public:
  // Create a log writer for the new log segment.
  static StatusWith<LogWriter *> New(LogManager *manager) {
    uint64_t newSegId = manager->files_.size() + 1;
    uint64_t newSegStart = manager->lastIndex_ + 1;
    std::string fname = manager->logsDir_ + "/" + SegmentFileName(newSegId, newSegStart);
    LOG(INFO) << fmt::format("creating new segment segId: {}, firstId: {}", newSegId, newSegStart);

    SegmentMetaData meta;
    meta.fileName = fname;
    meta.committed = false;

    WritableFile *wf;
    ASSIGN_IF_OK(Env::Default()->NewWritableFile(fname, Env::CREATE_NON_EXISTING), wf);

    return new LogWriter(wf, meta);
  }

  LogWriter(WritableFile *wf, SegmentMetaData meta) : file_(wf), meta_(meta) {}

  Status MarkCommitted(bool c) {
    char committed[1] = {static_cast<char>(c)};
    return file_->PositionedAppend(Slice(committed, 1), 0);
  }

  // Append log entries in range [begin, end) into the underlying segment.
  // If the current write is beyond the configured segment size, it returns a
  // iterator points at the next entry to be appended.
  StatusWith<ConstPBEntriesIterator> AppendEntries(ConstPBEntriesIterator begin,
                                                   ConstPBEntriesIterator end) {
    std::string rawEntries;
    ssize_t remains = FLAGS_log_segment_size - file_->Size();
    auto it = begin;
    for (; it != end; it++) {
      std::string entryBuf = EncodeToString(*it);
      remains -= entryBuf.size();
      rawEntries += std::move(entryBuf);
      if (remains <= 0) {
        it++;
        break;
      }
    }

    RETURN_NOT_OK(file_->Append(rawEntries));
    entriesNum_ += std::distance(begin, it);
    return it;
  }

  StatusWith<SegmentMetaData> Finish() {
    RETURN_NOT_OK(file_->Sync());
    RETURN_NOT_OK(file_->Close());
    return meta_;
  }

  bool Oversize() const {
    return FLAGS_log_segment_size <= file_->Size();
  }

  size_t TotalSize() const {
    return file_->Size();
  }

 private:
  friend class LogWriterTest;

 private:
  std::unique_ptr<WritableFile> file_;
  SegmentMetaData meta_;
  size_t entriesNum_;
};

}  // namespace wal
}  // namespace consensus