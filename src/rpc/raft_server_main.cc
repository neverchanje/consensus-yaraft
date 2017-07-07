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

#include "rpc/raft_server.h"

using namespace consensus::rpc;

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  ::sofa::pbrpc::RpcServerOptions opts;

  // using single-threaded server to avoid unnecessary data races in FSM.
  opts.work_thread_num = 1;

  ::sofa::pbrpc::RpcServer rpcServer(opts);
  auto raftService = RaftServiceImpl::New(&rpcServer);

  LOG(INFO) << "Start rpc server at: " << raftService->Url();
  if (!rpcServer.Start(raftService->Url())) {
    LOG(ERROR) << "Start rpc server failed";
    return EXIT_FAILURE;
  }

  if (!rpcServer.RegisterService(raftService)) {
    FMT_LOG(ERROR, "export service failed");
    return EXIT_FAILURE;
  }

  // Wait signal.
  rpcServer.Run();

  // Stop rpc server
  raftService->Stop();
  rpcServer.Stop();

  return 0;
}
