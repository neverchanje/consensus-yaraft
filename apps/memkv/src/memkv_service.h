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

#include <memory>

#include "db.h"
#include "memkv_store.h"
#include "pb/memkv.pb.h"

namespace memkv {

class MemKVServiceImpl : public pb::MemKVService {
 public:
  void Write(::google::protobuf::RpcController* controller,
             const ::memkv::pb::WriteRequest* request, ::memkv::pb::WriteResult* response,
             ::google::protobuf::Closure* done) override;

  void Read(::google::protobuf::RpcController* controller, const ::memkv::pb::ReadRequest* request,
            ::memkv::pb::ReadResult* response, ::google::protobuf::Closure* done) override;

  void Delete(::google::protobuf::RpcController* controller,
              const ::memkv::pb::DeleteRequest* request, ::memkv::pb::DeleteResult* response,
              ::google::protobuf::Closure* done) override;

  explicit MemKVServiceImpl(DB* db);

  ~MemKVServiceImpl() override;

 private:
  std::shared_ptr<DB> db_;
};

}  // namespace memkv