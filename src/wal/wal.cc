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

Status WriteAheadLog::Default(const WriteAheadLogOptions& options, WriteAheadLogUPtr* wal,
                              yaraft::MemStoreUptr* memstore) {
  LogManagerUPtr lm;
  RETURN_NOT_OK(LogManager::Recover(options, memstore, &lm));
  LOG_ASSERT(lm != nullptr);

  wal->reset(lm.release());
  return Status::OK();
}

WriteAheadLogOptions::WriteAheadLogOptions()
    : verify_checksum(true), log_segment_size(64 * 1024 * 1024) {}

WriteAheadLogUPtr TEST_CreateWalStore(const std::string& testDir, yaraft::MemStoreUptr* pMemstore) {
  WriteAheadLogOptions options;
  options.log_dir = testDir;
  options.verify_checksum = true;

  WriteAheadLogUPtr wal;
  FATAL_NOT_OK(WriteAheadLog::Default(options, &wal, pMemstore), "WriteAheadLog::Default");
  wal->Write({yaraft::pb::Entry()});

  return std::move(wal);
}

}  // namespace wal
}  // namespace consensus