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

#include <fmt/format.h>
#include <gflags/gflags.h>
#include <gtest/gtest.h>

namespace consensus {

using ::testing::FLAGS_gtest_random_seed;

class WritableFile;

#define ASSERT_OK(status)                        \
  do {                                           \
    const Status& _s = status;                   \
    if (_s) {                                    \
      SUCCEED();                                 \
    } else {                                     \
      FAIL() << "Bad status: " << _s.ToString(); \
    }                                            \
  } while (0)

class BaseTest : public ::testing::Test {
 public:
  BaseTest() : test_dir_(initTestDir()) {}

  virtual ~BaseTest() = default;

  std::string GetTestDir() {
    return test_dir_;
  }

  uint32_t SeedRandom();

  WritableFile* OpenFileForWrite(const std::string& fname,
                                 Env::CreateMode mode = Env::CREATE_IF_NON_EXISTING_TRUNCATE,
                                 bool sync_on_close = false);

 private:
  std::string initTestDir() {
    return fmt::format("/tmp/consensus-wal-test-{}/{}.{}", static_cast<int>(geteuid()),
                       testInfo()->test_case_name(), testInfo()->name());
  }

  const ::testing::TestInfo* const testInfo() {
    return ::testing::UnitTest::GetInstance()->current_test_info();
  }

 private:
  std::string test_dir_;
};

}  // namespace consensus