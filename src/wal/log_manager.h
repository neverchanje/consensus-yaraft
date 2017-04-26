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

#include "base/logging.h"
#include "base/status.h"
#include "wal/log_writer.h"
#include "wal/readable_log_segment.h"
#include "wal/segment_meta.h"
#include "wal/wal.h"

#include <silly/likely.h>

namespace consensus {

class WritableFile;

namespace wal {

class LogManager : public WriteAheadLog {
 public:
  LogManager() {}

  ~LogManager() {}

  // Required: no holes between logs and msg.entries.
  Status AppendEntries(const yaraft::pb::Message& msg) override {
    if (msg.entries().empty()) {
      return Status::OK();
    }

    if (!current_) {
      RETURN_NOT_OK(updateLogWriter());
    }

    uint64_t beginIdx = msg.entries().begin()->index();

    uint64_t segmentId;
    ASSIGN_IF_OK(FindSegmentId(beginIdx), segmentId);

    if (beginIdx <= lastIndex_) {
      RETURN_NOT_OK(files_[segmentId].TruncateLogAfterIndex(beginIdx));

      // Delete the following segments.
      for (uint64_t i = segmentId + 1; i < files_.size(); i++) {
        Env::Default()->DeleteFile(files_[i].fileName);
      }
    }

    RETURN_NOT_OK(doAppend(msg.entries().begin(), msg.entries().end()));
    return Status::OK();
  }

  StatusWith<size_t> FindSegmentId(uint64_t logIndex) const {
    if (files_.empty() || logIndex < files_.begin()->range.first) {
      return Status::Make(Error::LogCompacted, "");
    }

    if (logIndex > files_.rbegin()->range.second) {
      return Status::Make(Error::OutOfBound, "");
    }

    // TODO(optimize): use binary search
    for (size_t i = 0; i < files_.size(); i++) {
      const SegmentMetaData& meta = files_[i];
      if (logIndex <= meta.range.second && logIndex >= meta.range.first) {
        return i;
      }
    }
    // NotFound is a fatal error.
    return Status::Make(Error::NotFound, "");
  }

 private:
  // Append entries into log. It's guaranteed that there's no conflicted entry between MsgApp
  // and current log.
  Status doAppend(ConstPBEntriesIterator begin, ConstPBEntriesIterator end) {
    DLOG_ASSERT(!current_);

    auto it = begin;
    while (it != end) {
      ASSIGN_IF_OK(current_->AppendEntries(it, end), it);
      if (current_->Oversize()) {
        RETURN_NOT_OK(current_->Finish());
        delete current_.release();

        RETURN_NOT_OK(updateLogWriter());
      }
    }
    return Status::OK();
  }

  SegmentMetaData& lastSegmentMeta() {
    return *files_.rbegin();
  }

  Status updateLogWriter() {
    WritableFile* wf;
    ASSIGN_IF_OK(newSegment(), wf);
    current_.reset(new LogWriter(wf, &lastSegmentMeta()));
    return Status::OK();
  }

  // Create a new log segment and create a WritableFile for it.
  // NOTE: SegmentMetaData::fileSize will only be set after LogWriter finishing.
  StatusWith<WritableFile*> newSegment() {
    uint64_t newSegId = files_.size();
    uint64_t newSegStart = lastIndex_ + 1;
    std::string fname = SegmentFileName(newSegId, newSegStart);

    SegmentMetaData segMeta;
    segMeta.fileName = fname;
    segMeta.committed = false;
    segMeta.range = std::make_pair(newSegStart, newSegStart);
    files_.push_back(segMeta);

    WritableFile* wf;
    ASSIGN_IF_OK(Env::Default()->NewWritableFile(fname, Env::CREATE_NON_EXISTING), wf);
    return wf;
  }

 private:
  friend class LogManagerTest;

  std::unique_ptr<LogWriter> current_;
  std::vector<SegmentMetaData> files_;

  uint64_t lastIndex_;
};
}  // namespace wal
}  // namespace consensus