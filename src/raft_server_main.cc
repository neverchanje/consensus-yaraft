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

#include <mutex>

#include "raft_service.h"
#include "replicated_log.h"

#include "base/logging.h"

#include <boost/filesystem/operations.hpp>
#include <gflags/gflags.h>
#include <sofa/pbrpc/pbrpc.h>

using namespace consensus;

DEFINE_uint64(id, 1, "one of the values in {1, 2, 3}");
DEFINE_string(wal_dir, "", "directory to store wal");
DEFINE_int32(server_num, 3, "number of servers in the cluster");

ReplicatedLog* replicatedLog;
ReplicatedLogOptions options;
std::mutex writeMutex;

bool WriteServlet(const sofa::pbrpc::HTTPRequest& request, sofa::pbrpc::HTTPResponse& response) {
  writeMutex.lock();
  auto s = replicatedLog->Write(request.body->ToString());
  writeMutex.unlock();

  return response.content->Append(s.ToString());
}

bool InfoServlet(const sofa::pbrpc::HTTPRequest& request, sofa::pbrpc::HTTPResponse& response) {
  writeMutex.lock();
  auto info = replicatedLog->GetInfo();
  writeMutex.unlock();

  return response.content->Append(fmt::sprintf(
      R"({"commitIndex": %zu, "currentTerm": %zu, "logIndex": %zu, "currentLeader": %zu})",
      info.commitIndex, info.currentTerm, info.logIndex, info.currentLeader));
}

void InitReplicatedLog() {
  for (int i = 1; i <= FLAGS_server_num; i++) {
    options.initial_cluster[i] = fmt::format("127.0.0.1:{}", 12321 + i - 1);
  }
  options.id = FLAGS_id;
  options.wal_dir = FLAGS_wal_dir;
  options.heartbeat_interval = 100;
  options.election_timeout = 1000;
  options.taskQueue = new TaskQueue;

  {
    auto sw = ReplicatedLog::New(options);
    LOG_ASSERT(sw.IsOK()) << sw.ToString();
    replicatedLog = sw.GetValue();
  }
}

void DestroyResources() {
  delete options.taskQueue;
}

void PrintFlags() {
  FMT_LOG(INFO, "--id={}", FLAGS_id);
  FMT_LOG(INFO, "--wal_dir={}", FLAGS_wal_dir);
  FMT_LOG(INFO, "--server_num={}", FLAGS_server_num);
}

void ValidateFlags() {
  LOG_ASSERT(!FLAGS_wal_dir.empty());
}

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  PrintFlags();
  ValidateFlags();

  InitReplicatedLog();

  ::sofa::pbrpc::RpcServerOptions opts;
  ::sofa::pbrpc::RpcServer rpcServer(opts);

  std::string url = options.initial_cluster[options.id];
  LOG(INFO) << "start rpc server at: " << url;
  if (!rpcServer.Start(url)) {
    LOG(ERROR) << "start rpc server failed";
    return EXIT_FAILURE;
  }

  if (!rpcServer.RegisterService(replicatedLog->RaftServiceInstance(), false)) {
    FMT_LOG(ERROR, "export service failed");
    return EXIT_FAILURE;
  }

  rpcServer.RegisterWebServlet("/wal", sofa::pbrpc::NewPermanentExtClosure(&WriteServlet));
  rpcServer.RegisterWebServlet("/info", sofa::pbrpc::NewPermanentExtClosure(&InfoServlet));

  // Wait signal.
  rpcServer.Run();

  rpcServer.Stop();

  DestroyResources();

  return 0;
}