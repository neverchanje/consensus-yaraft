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

#include "base/endianness.h"

#include <gflags/gflags.h>
#include <google/protobuf/io/coded_stream.h>

namespace consensus {

#if defined(PROTOBUF_LITTLE_ENDIAN)
DEFINE_bool(is_little_endian, true, "");
#else
DEFINE_bool(is_little_endian, false, "");
#endif

}  // namespace consensus
