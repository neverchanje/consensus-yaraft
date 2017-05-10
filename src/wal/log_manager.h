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
#include "wal/wal.h"

namespace consensus {

class WritableFile;

namespace wal {

class SegmentMetaData;
class LogWriter;

class LogManager : public WriteAheadLog {
 public:
  explicit LogManager(const Slice& logsDir);

  ~LogManager();

  // Required: no holes between logs and msg.entries.
  Status AppendEntries(const yaraft::pb::Message& msg) override;

  StatusWith<size_t> FindSegmentId(uint64_t logIndex) const;

  // the number of log segments
  size_t SegmentNum() const {
    return files_.size() + static_cast<size_t>(bool(current_));
  }

 private:
  // Append entries into log. It's guaranteed that there's no conflicted entry between MsgApp
  // and current log.
  Status doAppend(ConstPBEntriesIterator begin, ConstPBEntriesIterator end);

  Status updateLogWriter();

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

  std::string logsDir_;
};
}  // namespace wal
}  // namespace consensus