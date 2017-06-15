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

#include "base/status.h"

#include <fmt/format.h>

namespace consensus {

#define DUMB_ERROR_TO_STRING(err) \
  case (err):                     \
    return #err

std::string Error::toString(unsigned int code) {
  switch (code) {
    DUMB_ERROR_TO_STRING(IOError);
    DUMB_ERROR_TO_STRING(NotSupported);
    DUMB_ERROR_TO_STRING(Corruption);
    DUMB_ERROR_TO_STRING(LogCompacted);
    DUMB_ERROR_TO_STRING(OutOfBound);
    DUMB_ERROR_TO_STRING(YARaftERR);
    DUMB_ERROR_TO_STRING(BadConfig);
    default:
      return fmt::format("Unknown error codes: {}", code);
  }
}

}  // namespace consensus
