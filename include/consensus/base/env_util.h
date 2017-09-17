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

#include <cstddef>
#include <cstdint>

#include "consensus/base/slice.h"
#include "consensus/base/status.h"

namespace consensus {

class RandomAccessFile;

namespace env_util {

// Read the full content of `file` into `scratch`.
Status ReadFully(RandomAccessFile *file, uint64_t offset, size_t n, Slice *result, char *scratch);

// Read data into an unallocated buffer.
Status ReadFullyToBuffer(const Slice &fname, Slice *result, char **scratch);

}  // namespace env_util
}  // namespace consensus