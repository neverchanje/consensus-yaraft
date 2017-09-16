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

#include "status.h"

#include <glog/logging.h>

namespace memkv {

#define ERROR_TO_STRING(err) \
  case (err):                \
    return #err

std::string Error::toString(unsigned int c) {
  auto code = static_cast<ErrorCodes>(c);
  switch (code) {
    ERROR_TO_STRING(OK);
    ERROR_TO_STRING(InvalidArgument);
    ERROR_TO_STRING(NodeNotExist);
    default:
      LOG(FATAL) << "invalid error code: " << c;
      assert(false);
  }
}

}  // namespace memkv