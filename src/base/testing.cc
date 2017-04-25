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

#include <chrono>

#include "base/env.h"
#include "base/testing.h"
#include "random.h"

#include <glog/logging.h>

namespace consensus {

uint32_t BaseTest::SeedRandom() {
  int seed;
  if (FLAGS_gtest_random_seed) {
    seed = FLAGS_gtest_random_seed;
  } else {
    seed = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count() / 1e9);
  }
  LOG(INFO) << "Using random seed: " << seed;
  return static_cast<uint32_t>(seed);
}

WritableFile* BaseTest::OpenFileForWrite(const std::string& fname, Env::CreateMode mode,
                                         bool sync_on_close) {
  auto sw = Env::Default()->NewWritableFile(fname, mode, sync_on_close);
  CHECK(sw.GetStatus().IsOK()) << sw.GetStatus();
  return sw.GetValue();
}

void BaseTest::WriteTestFile(const Slice& path, size_t size, std::string* testData, Random* rng) {
  auto sw = Env::Default()->NewWritableFile(path);
  ASSERT_OK(sw.GetStatus());
  std::unique_ptr<WritableFile> wf(sw.GetValue());

  (*testData) = RandomString(size, rng);
  ASSERT_OK(wf->Append(*testData));
  ASSERT_OK(wf->Close());
}

}  // namespace consensus