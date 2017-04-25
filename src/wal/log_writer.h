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
#include "wal/options.h"
#include "wal/wal.h"

#include <silly/likely.h>

namespace consensus {
namespace wal {

std::string EncodeToString(const yaraft::pb::Entry &entry);
void EncodedToArray(const yaraft::pb::Entry &entry, char **result, size_t *len);
void EncodedToAllocatedArray(const yaraft::pb::Entry &entry, char *result, size_t *len);

struct LogWriter {
 public:
  explicit LogWriter(WritableFile *f) : file_(f) {}

  Status MarkCommitted(bool c) {
    char committed[1] = {static_cast<char>(c)};
    return file_->PositionedAppend(Slice(committed, 1), 0);
  }

  // Append log entries in range [begin, end) into the underlying segment.
  // If current write is beyond the configured segment size, it returns a
  // iterator points at the last entry appended.
  StatusWith<ConstPBEntriesIterator> AppendEntries(ConstPBEntriesIterator begin,
                                                   ConstPBEntriesIterator end) {
    std::string rawEntries;
    size_t remains = FLAGS_log_segment_size - file_->Size();
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
    return it;
  }

  Status Finish() {
    return file_->Close();
  }

  bool Oversize() {
    return FLAGS_log_segment_size <= file_->Size();
  }

 private:
  std::unique_ptr<WritableFile> file_;
};

}  // namespace wal
}  // namespace consensus