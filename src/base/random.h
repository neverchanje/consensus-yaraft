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

#include <random>
#include <string>

namespace consensus {

class Random {
 public:
  explicit Random(uint32_t seed) : gen_(seed) {}

  // Generates the next random number
  uint32_t Next() {
    return static_cast<uint32_t>(gen_());
  }

  // Returns a uniformly distributed value in the range [0..n-1]
  // REQUIRES: n > 0
  uint32_t Uniform(uint32_t n) {
    return Next() % n;
  }

 private:
  std::mt19937 gen_;
};

void RandomString(void* dest, size_t n, Random* rng);

inline std::string RandomString(size_t n, Random* rng) {
  std::string s;
  s.resize(n);
  RandomString(&s[0], n, rng);
  return s;
}

}  // namespace consensus