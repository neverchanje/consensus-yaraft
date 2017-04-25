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

#include "base/env.h"
#include "base/env_util.h"
#include "base/random.h"
#include "base/testing.h"

using namespace consensus;
using namespace std;

class TestEnv : public BaseTest {
 public:
  TestEnv() : rng_(SeedRandom()) {}

  void TestAppendVector(size_t num_slices, size_t slice_size) {
    const string kTestPath = GetTestDir() + "/test_env_appendvec_read_append";

    unique_ptr<WritableFile> wf(OpenFileForWrite(kTestPath));

    vector<Slice> slices(num_slices);
    LOG(INFO) << fmt::format(
        "appending a vector of slices(number of slices={}, size of slice={} b)", num_slices,
        slice_size);

    vector<string> dataSet;
    RandomDataSet(num_slices, slice_size, &dataSet);

    for (int i = 0; i < num_slices; i++) {
      slices[i] = dataSet[i];
    }

    ASSERT_OK(wf->AppendVector(slices));
    ASSERT_OK(wf->Close());

    string testData;
    for (string& s : dataSet) {
      testData += s;
    }
    ASSERT_EQ(testData.size(), num_slices * slice_size);

    ReadAndVerifyTestData(kTestPath, testData);
  }

  void ReadAndVerifyTestData(const string& filePath, const string& testData) {
    Slice s;
    std::string scratch;
    ASSERT_OK(env_util::ReadFully(filePath, &s, &scratch));

    ASSERT_EQ(s.data(), scratch.data());
    ASSERT_EQ(testData, scratch);
  }

  void RandomDataSet(size_t num_slices, size_t slice_size, vector<string>* dataSet) {
    dataSet->resize(num_slices);
    for (int i = 0; i < num_slices; i++) {
      (*dataSet)[i] = RandomString(slice_size, &rng_);
    }
  }

 protected:
  Random rng_;
};

TEST_F(TestEnv, ReadFully) {
  ASSERT_OK(Env::Default()->CreateDirIfMissing(GetTestDir()));

  const int kFileSize = 64 * 1024;
  const int kTrialNum = 1000;
  const string kTestPath = GetTestDir() + "/test";

  for (int i = 0; i < kTrialNum; i++) {
    string testData;
    WriteTestFile(kTestPath, kFileSize, &testData, &rng_);
    ReadAndVerifyTestData(kTestPath, testData);
    Env::Default()->DeleteFile(kTestPath);
    ASSERT_NO_FATAL_FAILURE();
  }
  ASSERT_OK(Env::Default()->DeleteRecursively(GetTestDir()));
}

TEST_F(TestEnv, AppendVector) {
  ASSERT_OK(Env::Default()->CreateDirIfMissing(GetTestDir()));
  TestAppendVector(2000, 1024);
  ASSERT_OK(Env::Default()->DeleteRecursively(GetTestDir()));
}