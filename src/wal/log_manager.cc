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

#include "wal/log_manager.h"
#include "base/logging.h"
#include "wal/log_writer.h"
#include "wal/readable_log_segment.h"
#include "wal/segment_meta.h"

#include <fmt/format.h>
#include <silly/likely.h>

namespace consensus {
namespace wal {

LogManager::LogManager(const Slice& logsDir)
    : lastIndex_(0), empty_(true), logsDir_(logsDir.ToString()) {}

LogManager::~LogManager() {}

Status LogManager::AppendEntries(const yaraft::pb::Message& msg) {
  if (msg.entries().empty()) {
    return Status::OK();
  }

  uint64_t beginIdx = msg.entries().begin()->index();
  if (empty_) {
    lastIndex_ = beginIdx - 1;  // start at the first entry received.
    empty_ = false;
  }

  // the overlapped part of logs will not be deleted until snapshotting.
  return doAppend(msg.entries().begin(), msg.entries().end());
}

StatusWith<size_t> LogManager::FindSegmentId(uint64_t logIndex) const {
  if (files_.empty() || logIndex < files_.begin()->range.first) {
    return Status::Make(Error::LogCompacted, "");
  }

  if (current_ && logIndex > current_->LastIndex()) {
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

Status LogManager::doAppend(ConstPBEntriesIterator begin, ConstPBEntriesIterator end) {
  auto segStart = begin;
  auto it = begin;
  while (true) {
    if (!current_) {
      LogWriter* w;
      ASSIGN_IF_OK(LogWriter::New(this), w);
      current_.reset(w);
    }

    ASSIGN_IF_OK(current_->AppendEntries(segStart, end), it);
    lastIndex_ += std::distance(segStart, it);

    if (it == end) {
      break;
    }

    SegmentMetaData meta;
    ASSIGN_IF_OK(current_->Finish(), meta);
    delete current_.release();
    files_.push_back(meta);

    segStart = it;
  }
  return Status::OK();
}

}  // namespace wal
}  // namespace consensus