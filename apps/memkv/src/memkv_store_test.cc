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

#include <unordered_map>

#include "memkv_store.h"
#include "testing.h"

#include <consensus/base/random.h>

using namespace memkv;

class TestMemKV : public testing::Test {
 public:
};

// This test ensures that the same keys can always read
// the same value.
TEST_F(TestMemKV, WriteAndGet) {
  struct TestData {
    std::string key;
    std::string testKey;

    Error::ErrorCodes wcode;
  } tests[] = {
      {"/tmp/xiaomi/ads/pegasus-1/table-num-counter", "/tmp/xiaomi/ads/pegasus-1/table-num-counter",
       Error::OK},
      {"/tmp/xiaomi/ads/pegasus-1/table-num-counter", "/var", Error::NodeNotExist},
      {"tmp/xiaomi", "///tmp/xiaomi", Error::OK},

      // can not pass an empty path as parameter
      {"tmp", "", Error::InvalidArgument},
  };

  for (auto t : tests) {
    MemKvStore kv;

    std::string expected = "test_value";
    ASSERT_OK(kv.Write(t.key, expected));
    std::string actual;
    ASSERT_ERROR(kv.Get(t.testKey, &actual), t.wcode);
    if (t.wcode == Error::OK) {
      ASSERT_EQ(actual, expected);
    }
  }
}

std::string randomString(size_t len, consensus::Random *rnd) {
  std::string result;
  result.resize(len);
  for (size_t i = 0; i < len; i++) {
    result[i] = static_cast<char>('a' + rnd->Uniform(26));
  }
  return result;
}

TEST_F(TestMemKV, MultiReadWrite) {
  std::unordered_map<std::string, std::string> data_map;
  consensus::Random rnd(0);

  for (int i = 0; i < 2; i++) {
    std::string key = randomString(10, &rnd);
    data_map[key] = randomString(20, &rnd);
    LOG(INFO) << key << " " << data_map[key];
  }

  MemKvStore store;
  for (auto &kv : data_map) {
    store.Write(kv.first, kv.second);
  }

  for (auto &kv : data_map) {
    std::string value;
    ASSERT_OK(store.Get(kv.first, &value));
    ASSERT_EQ(value, kv.second);
  }
}

TEST_F(TestMemKV, Delete) {
  MemKvStore kv;
  std::string key = "/tmp", expected = "1";
  ASSERT_OK(kv.Write(key, expected));

  std::string actual;
  ASSERT_OK(kv.Get(key, &actual));
  ASSERT_EQ(actual, expected);

  ASSERT_OK(kv.Delete(key));
  ASSERT_ERROR(kv.Get(key, &actual), Error::NodeNotExist);
}

TEST_F(TestMemKV, DeleteRecursively) {
  MemKvStore kv;
  std::string expected = "1";
  ASSERT_OK(kv.Write("/tmp/xiaomi/ads/pegasus-1/table-num-counter", expected));

  std::string actual;
  ASSERT_OK(kv.Get("/tmp/xiaomi/ads/pegasus-1/table-num-counter", &actual));
  ASSERT_EQ(actual, expected);

  ASSERT_OK(kv.Delete("/tmp/xiaomi/ads/pegasus-1"));
  ASSERT_OK(kv.Get("/tmp/xiaomi/ads", &actual));
  ASSERT_ERROR(kv.Get("/tmp/xiaomi/ads/pegasus-1", &actual), Error::NodeNotExist);
}