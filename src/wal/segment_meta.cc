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

#include "wal/segment_meta.h"
#include "base/env.h"
#include "base/logging.h"
#include "wal/readable_log_segment.h"

#include <fmt/format.h>

namespace consensus {
namespace wal {

Status Truncate(const std::string& fileName, size_t size) {
  WritableFile* wf;
  ASSIGN_IF_OK(Env::Default()->NewWritableFile(fileName, Env::CREATE_NON_EXISTING), wf);
  RETURN_NOT_OK(wf->Truncate(size));
  RETURN_NOT_OK(wf->Close());
  return Status::OK();
}

std::string SegmentFileName(uint64_t segmentId, uint64_t firstIdx) {
  return fmt::format("{}-{}.wal", segmentId, firstIdx);
}

Status SegmentMetaData::TruncateLogAfterIndex(uint64_t logIndex) {
  ReadableLogSegment* seg;
  ASSIGN_IF_OK(ReadableLogSegment::Create(this), seg);
  std::unique_ptr<ReadableLogSegment> d(seg);

  RETURN_NOT_OK(seg->SkipHeader());
  while (true) {
    yaraft::pb::Entry ent;
    ASSIGN_IF_OK(seg->ReadEntry(), ent);

    if (ent.index() == logIndex) {
      RETURN_NOT_OK(Truncate(fileName, seg->Offset()));
      break;
    }
  }
  return Status::OK();
}

}  // namespace wal
}  // namespace consensus