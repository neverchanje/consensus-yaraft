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

namespace consensus {
namespace wal {

using silly::operator""_sl;
static constexpr Slice kLogSegmentHeaderMagic = "yaraft_log"_sl;

struct SegmentMetaData {
  std::string fileName;
  size_t numEntries;

  SegmentMetaData() : numEntries(0){};
};

std::string SegmentFileName(uint64_t segmentId, uint64_t firstIdx);

}  // namespace wal
}  // namespace consensus