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
class Random;

#define ASSERT_OK(status)                        \
  do {                                           \
    const auto& _s = status;                     \
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

class BaseTest : public ::testing::Test {
 public:
  struct TestDirectoryGuard {
    explicit TestDirectoryGuard(const BaseTest* base) : base_(base) {
      acquire();
    }

    ~TestDirectoryGuard() {
      release();
    }

   private:
    void acquire() {
      ASSERT_OK(Env::Default()->CreateDirIfMissing(base_->GetTestDir()));
    }

    void release() {
      ASSERT_OK(Env::Default()->DeleteRecursively(base_->GetParentDir()));
    }

   private:
    const BaseTest* base_;
  };

 public:
  BaseTest() {
    initTestDir();
  }

  virtual ~BaseTest() = default;

  std::string GetTestDir() const {
    return test_dir_;
  }

  std::string GetParentDir() const {
    return parent_dir_;
  }

  uint32_t SeedRandom();

  WritableFile* OpenFileForWrite(const std::string& fname,
                                 Env::CreateMode mode = Env::CREATE_IF_NON_EXISTING_TRUNCATE,
                                 bool sync_on_close = false);

  // Write 'size' bytes of data to a file, with a simple pattern stored in it.
  void WriteTestFile(const Slice& path, size_t size, std::string* testData, Random* rng);

  TestDirectoryGuard* CreateTestDirGuard() const {
    return new TestDirectoryGuard(this);
  }

 private:
  void initTestDir() {
    parent_dir_ = fmt::format("/tmp/consensus-yaraft-test-{}", static_cast<int>(geteuid()));
    test_dir_ =
        parent_dir_ + fmt::format("/{}.{}", testInfo()->test_case_name(), testInfo()->name());
  }

  const ::testing::TestInfo* const testInfo() {
    return ::testing::UnitTest::GetInstance()->current_test_info();
  }

 private:
  std::string test_dir_;
  std::string parent_dir_;
};

typedef std::unique_ptr<BaseTest::TestDirectoryGuard> TestDirGuard;

}  // namespace consensus