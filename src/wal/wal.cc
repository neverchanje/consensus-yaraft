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

#include "wal/wal.h"
#include "base/logging.h"
#include "wal/log_manager.h"

namespace consensus {
namespace wal {

Status WriteAheadLog::Default(const WriteAheadLogOptions& options, WriteAheadLog** wal,
                              yaraft::MemoryStorage* memstore) {
  LogManager* lm;
  ASSIGN_IF_OK(LogManager::Recover(options, memstore), lm);
  *wal = lm;

  LOG_ASSERT(lm != nullptr);

  return Status::OK();
}

WriteAheadLogOptions::WriteAheadLogOptions()
    : verify_checksum(true), log_segment_size(64 * 1024 * 1024) {}

WriteAheadLog* TEST_GetWalStore(const std::string& testDir, yaraft::MemoryStorage* memstore) {
  WriteAheadLogOptions options;
  options.log_dir = testDir;

  // automatically delete the memstore
  std::unique_ptr<yaraft::MemoryStorage> d;
  if (memstore == nullptr) {
    memstore = new yaraft::MemoryStorage();
    d.reset(memstore);
  }

  WriteAheadLog* wal = nullptr;
  FATAL_NOT_OK(WriteAheadLog::Default(options, &wal, memstore), "WriteAheadLog::Default");

  wal->Write({yaraft::pb::Entry()});

  return wal;
}

}  // namespace wal
}  // namespace consensus