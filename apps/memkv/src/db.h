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

#include <map>

#include "slice.h"
#include "status.h"

#include <brpc/server.h>
#include <consensus/raft_service.h>

namespace memkv {

// Abstract handle to particular state of a DB.
// A Snapshot is an immutable object and can therefore be safely
// accessed from multiple threads without any external synchronization.
class Snapshot {
 protected:
  virtual ~Snapshot();
};

class DBOptions {
 public:
  uint64_t member_id;
  std::string wal_dir;
  std::map<uint64_t, std::string> initial_cluster;
};

class DB {
 public:
  static StatusWith<DB *> Bootstrap(const DBOptions &options);

  Status Write(const Slice &path, const Slice &value);

  Status Delete(const Slice &path);

  Status Get(const Slice &path, bool stale, std::string *data);

  consensus::pb::RaftService *CreateRaftServiceInstance() const;

  DB();

  ~DB();

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace memkv