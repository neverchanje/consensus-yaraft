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

#include "db.h"
#include "logging.h"
#include "memkv_service.h"

#include <boost/make_unique.hpp>
#include <consensus/base/env.h>
#include <consensus/base/glog_logger.h>

using namespace memkv;

DEFINE_uint64(id, 1, "one of the values in {1, 2, 3}");
DEFINE_string(wal_dir, "", "directory to store wal");
DEFINE_int32(server_count, 3, "number of servers in the cluster");
DEFINE_string(memkv_log_dir, "",
              "If specified, logfiles are written into this directory instead "
              "of the default logging directory.");

void InitLogging(const char* argv0) {
  google::InitGoogleLogging(argv0);
  FLAGS_log_dir = FLAGS_memkv_log_dir;
  if (!FLAGS_log_dir.empty()) {
    FATAL_NOT_OK(consensus::Env::Default()->CreateDirIfMissing(FLAGS_log_dir), FLAGS_log_dir);
  } else {
    // print to stderr when log_dir is not specified.
    FLAGS_logtostderr = true;
  }
  yaraft::SetLogger(boost::make_unique<consensus::GLogLogger>());
}

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  InitLogging(argv[0]);

  //
  // -- create DB instance --
  //
  DBOptions options;
  options.member_id = FLAGS_id;
  options.wal_dir = FLAGS_wal_dir;
  for (int i = 1; i <= FLAGS_server_count; i++) {
    // TODO: initial_cluster should be configured by user
    options.initial_cluster[i] = fmt::format("127.0.0.1:{}", 12320 + i);
  }
  auto sw = DB::Bootstrap(options);
  if (!sw.IsOK()) {
    LOG(FATAL) << sw.GetStatus();
  }
  DB* db = sw.GetValue();

  //
  // -- start memkv server --
  //
  FMT_LOG(INFO, "Starting memkv server {} at {}", FLAGS_id, options.initial_cluster[FLAGS_id]);
  FMT_LOG(INFO, "--wal_dir: {}", FLAGS_wal_dir);
  brpc::ServerOptions opts;
  opts.num_threads = 1;
  brpc::Server server;
  server.AddService(new MemKVServiceImpl(db), brpc::SERVER_OWNS_SERVICE);
  server.AddService(db->CreateRaftServiceInstance(), brpc::SERVER_OWNS_SERVICE);
  server.Start(options.initial_cluster[FLAGS_id].c_str(), &opts);
  server.RunUntilAskedToQuit();

  return 0;
}