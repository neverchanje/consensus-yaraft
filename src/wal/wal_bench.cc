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

#include "base/logging.h"
#include "base/testing.h"
#include "wal/wal.h"

#include <benchmark/benchmark.h>

using yaraft::EntryVec;

using namespace consensus;
using namespace consensus::wal;

void WalBench(benchmark::State& state) {
  TestDirectoryHelper dirHelper("/tmp/consensus-wal-bench");
  std::unique_ptr<WriteAheadLog> wal(TEST_GetWalStore(dirHelper.GetTestDir()));

  size_t per_size = state.range(0);
  int num_entries = state.range(1);

  // prepare data for writing
  size_t totalBytes = 0;
  std::string data = std::string(per_size, 'a');

  EntryVec entries;
  for (uint64_t i = 0; i < num_entries; i++) {
    entries.push_back(yaraft::PBEntry().Index(i + 1).Term(1).Data(data).v);
    totalBytes += entries.back().ByteSize();
  }

  // start benchmark
  while (state.KeepRunning()) {
    FATAL_NOT_OK(wal->Write(entries), "WriteAheadLog::Write");
  }

  state.SetBytesProcessed(state.iterations() * totalBytes);
}

BENCHMARK(WalBench)
    ->Args({1000, 1})
    ->Args({1000, 10})
    ->Args({1000, 100})
    ->Args({1000, 500})
    ->Args({1000, 1000})
    ->Args({10000, 1})
    ->Args({10000, 10})
    ->Args({10000, 100})
    ->Args({10000, 500})
    ->Args({10000, 1000})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();