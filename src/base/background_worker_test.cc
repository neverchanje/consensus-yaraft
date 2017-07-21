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

#include "base/background_worker.h"
#include "base/testing.h"

using namespace consensus;

// This test verifies BackgroundWorker::Stop will stop the background thread only when it's
// stoppable, otherwise it generates fault prompt.
TEST(BackgroundWorkerTest, Stop) {
  BackgroundWorker worker;
  auto func = []() { usleep(1000 * 1); };

  ASSERT_EQ(worker.Stop().Code(), Error::RuntimeError);

  {
    worker.StartLoop(func);
    ASSERT_OK(worker.Stop());
  }

  {
    worker.StartLoop(func);
    ASSERT_OK(worker.Stop());
    ASSERT_EQ(worker.Stop().Code(), Error::RuntimeError);
  }
}

TEST(BackgroundWorkerTest, Start) {
  BackgroundWorker worker;

  {
    auto func = []() { usleep(1000 * 1); };
    worker.StartLoop(func);
    ASSERT_EQ(worker.StartLoop(func).Code(), Error::RuntimeError);
    worker.Stop();
  }
}