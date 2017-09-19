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

#include "memkv_store.h"
#include "testing.h"

using namespace memkv;

class TestMemKV : public testing::Test {
 public:
};

TEST_F(TestMemKV, WriteAndGet) {
  MemKvStore kv;
  std::string expected = "1";

  ASSERT_OK(kv.Write("/tmp/xiaomi/ads/pegasus-1/table-num-counter", expected));

  std::string actual;
  ASSERT_OK(kv.Get("/tmp/xiaomi/ads/pegasus-1/table-num-counter", &actual));
  ASSERT_EQ(actual, expected);

  ASSERT_ERROR(kv.Get("/var", &actual), Error::NodeNotExist);
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