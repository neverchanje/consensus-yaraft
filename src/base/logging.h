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

#include <glog/logging.h>

namespace consensus {

/// @brief Emit a warning if @c to_call returns a bad status.
#define WARN_NOT_OK(to_call, warning_prefix)                     \
  do {                                                           \
    const ::consensus::Status& _s = (to_call);                   \
    if (PREDICT_FALSE(!_s.OK())) {                               \
      LOG(WARNING) << (warning_prefix) << ": " << _s.ToString(); \
    }                                                            \
  } while (0);

/// @brief Return the given status if it is not @c OK.
#define RETURN_NOT_OK SILLY_RETURN_NOT_OK

}  // namespace consensus