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

#include "base/env_util.h"
#include "base/env.h"
#include "base/logging.h"

#include <fmt/format.h>
#include <silly/likely.h>

namespace consensus {
namespace env_util {

// Read the full content of `file` into `scratch`.
// If success, the result must have length n.
Status ReadFully(RandomAccessFile *file, uint64_t offset, size_t n, Slice *result, char *scratch) {
  size_t remain = n;
  char *dst = scratch;
  while (remain > 0) {
    Slice this_result;
    RETURN_NOT_OK(file->Read(offset, remain, &this_result, dst));
    DCHECK_LE(this_result.size(), remain);
    if (this_result.size() == 0) {
      return Status::Make(Error::IOError,
                          fmt::format("EOF trying to read {} bytes at offset {}", n, offset));
    }
    dst += this_result.size();
    remain -= this_result.size();
    offset += this_result.size();
  }
  *result = Slice(scratch, n - remain);
  return Status::OK();
}

Status ReadFully(const Slice &fname, Slice *result, std::string *scratch) {
  RandomAccessFile *raf;
  ASSIGN_IF_OK(Env::Default()->NewRandomAccessFile(fname), raf);

  size_t n;
  ASSIGN_IF_OK(raf->Size(), n);
  scratch->resize(n);

  return ReadFully(raf, 0, n, result, &(*scratch)[0]);
}

}  // namespace env_util
}  // namespace consensus