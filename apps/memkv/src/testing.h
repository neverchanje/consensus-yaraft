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

#include "logging.h"

#include <gtest/gtest.h>

#define ASSERT_OK(status)                        \
  do {                                           \
    const auto &_s = status;                     \
    if (_s.IsOK()) {                             \
      SUCCEED();                                 \
    } else {                                     \
      FAIL() << "Bad status: " << _s.ToString(); \
    }                                            \
  } while (0)

#define ASSIGN_IF_ASSERT_OK(statusWith, var) \
  do {                                       \
    auto _sw = statusWith;                   \
    ASSERT_OK(_sw);                          \
    (var) = _sw.GetValue();                  \
  } while (0)

#define ASSERT_ERROR(status, err)                                                        \
  do {                                                                                   \
    const auto &_s = status;                                                             \
    if (_s.Code() == err) {                                                              \
      SUCCEED();                                                                         \
    } else {                                                                             \
      FAIL() << "expected error is: " << Error::toString(static_cast<unsigned int>(err)) \
             << ", actual error is: " << _s.ToString();                                  \
    }                                                                                    \
  } while (0)
