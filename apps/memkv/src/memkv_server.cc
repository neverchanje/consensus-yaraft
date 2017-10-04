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

#include "logging.h"
#include "memkv_service.h"

#include <brpc/server.h>

using namespace memkv;

DEFINE_uint64(id, 1, "one of the values in {1, 2, 3}");
DEFINE_string(wal_dir, "", "directory to store wal");
DEFINE_int32(server_num, 3, "number of servers in the cluster");

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  DBOptions options;
  options.member_id = FLAGS_id;
  options.wal_dir = FLAGS_wal_dir;

  auto sw = DB::Bootstrap(options);
  if (!sw.IsOK()) {
    LOG(FATAL) << sw.GetStatus();
  }

  DB* db = sw.GetValue();

  brpc::ServerOptions opts;
  brpc::Server server;
  server.AddService(new MemKVServiceImpl(db), brpc::SERVER_OWNS_SERVICE, "/");
  server.Start(static_cast<int>(12345 + FLAGS_id), &opts);

  return 0;
}