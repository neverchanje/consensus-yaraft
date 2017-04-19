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

  void TestAppendVector(size_t num_slices, size_t slice_size, Env::CreateMode mode,
                        bool sync_on_close) {
    const string kTestPath = GetTestDir() + "/test_env_appendvec_read_append";

    auto sw = Env::Default()->NewWritableFile(kTestPath, mode, sync_on_close);
    ASSERT_OK(sw.GetStatus());
    unique_ptr<WritableFile> file(sw.GetValue());

    vector<Slice> slices(num_slices);
    LOG(INFO) << fmt::format(
        "appending a vector of slices(number of slices={}, size of slice={} b)", num_slices,
        slice_size);

    vector<string> dataSet;
    RandomDataSet(num_slices, slice_size, &dataSet);

    for (int i = 0; i < num_slices; i++) {
      slices[i] = dataSet[i];
    }

    ASSERT_OK(file->AppendVector(slices));

    // Verify the entire file
    ASSERT_OK(file->Close());

  }

  void ReadAndVerifyTestData(const string& filePath, size_t offset, size_t n,
                             const string& testData) {
    // Reopen for read
    auto sw = Env::Default()->NewRandomAccessFile(filePath);
    ASSERT_OK(sw.GetStatus());
    unique_ptr<RandomAccessFile> raf(sw.GetValue());

    ReadAndVerifyTestData(raf.get(), offset, n, testData);
  }


  void ReadAndVerifyTestData(RandomAccessFile* raf, size_t offset, size_t n,
                             const string& testData) {
    string scratch(n, '\0');
    Slice s;
    ASSERT_OK(env_util::ReadFully(raf, offset, n, &s, &scratch[0]));
    ASSERT_EQ(s.data(), scratch.data());
    ASSERT_EQ(n, s.size());
    ASSERT_EQ(testData, scratch);
  }

  void RandomDataSet(size_t num_slices, size_t slice_size, vector<string>* dataSet) {
    dataSet->resize(num_slices);
    for (int i = 0; i < num_slices; i++) {
      (*dataSet)[i] = RandomString(slice_size, &rng_);
    }
  }

  // Write 'size' bytes of data to a file, with a simple pattern stored in it.
  void WriteTestFile(const string& path, size_t size, string* testData) {
    auto sw = Env::Default()->NewWritableFile(path);
    ASSERT_OK(sw.GetStatus());
    unique_ptr<WritableFile> wf(sw.GetValue());

    (*testData) = RandomString(size, &rng_);
    ASSERT_OK(wf->Append(Slice(*testData)));
    ASSERT_OK(wf->Close());
  }

 private:
  Random rng_;
};

TEST_F(TestEnv, ReadFully) {
  ASSERT_OK(Env::Default()->CreateDirIfMissing(GetTestDir()));

  const int kFileSize = 64 * 1024;
  const string kTestPath = GetTestDir() + "/test";

  string testData;
  WriteTestFile(kTestPath, kFileSize, &testData);
  ASSERT_NO_FATAL_FAILURE();
  ReadAndVerifyTestData(kTestPath, 0, kFileSize, testData);

  ASSERT_OK(Env::Default()->DeleteRecursively(GetTestDir()));
}

TEST_F(TestEnv, AppendVector) {

}