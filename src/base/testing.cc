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

#include "base/testing.h"

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

}  // namespace consensus