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

#include "memkv_service.h"
#include "logging.h"

namespace memkv {

static inline pb::ErrCode memkvErrorToRpcErrno(Error::ErrorCodes code) {
  switch (code) {
    case Error::OK:
      return pb::OK;
    case Error::InvalidArgument:
      return pb::InvalidArgument;
    case Error::NodeNotExist:
      return pb::NodeNotExist;
    case Error::ConsensusError:
      return pb::ConsensusError;
    default:
      LOG(FATAL) << "Unexpected error code: " << Error::toString(code);
      return pb::OK;
  }
}

// write request via http goes like this:
//  http 'URL:PORT/Write/abc/a?value=hello'
// if ok, path "/abc/a" will be set to "hello"
void MemKVServiceImpl::Write(::google::protobuf::RpcController *controller,
                             const ::memkv::pb::WriteRequest *request,
                             ::memkv::pb::WriteResult *response,
                             ::google::protobuf::Closure *done) {
  auto cntl = static_cast<brpc::Controller *>(controller);
  Status s;
  if (cntl->has_http_request()) {
    Slice path(cntl->http_request().unresolved_path());
    Slice value(*cntl->http_request().uri().GetQuery("value"));
    s = db_->Write(path, value);
  } else {
    s = db_->Write(request->path(), request->value());
  }

  response->set_errorcode(memkvErrorToRpcErrno(s.Code()));
  if (!s.IsOK()) {
    response->set_errormessage(s.ToString());
  }
  done->Run();
}

void MemKVServiceImpl::Read(::google::protobuf::RpcController *controller,
                            const ::memkv::pb::ReadRequest *request,
                            ::memkv::pb::ReadResult *response, ::google::protobuf::Closure *done) {
  auto cntl = static_cast<brpc::Controller *>(controller);
  auto result = new std::string;
  Status s;
  if (cntl->has_http_request()) {
    // by default brpc will encode string to base64
    // we don't need this feature here.
    cntl->set_pb_bytes_to_base64(false);

    Slice path(cntl->http_request().unresolved_path());
    bool stale = false;
    if (cntl->http_request().uri().GetQuery("stale") != nullptr) {
      stale = true;
    }
    s = db_->Get(path, stale, result);
  } else {
    s = db_->Get(request->path(), request->stale(), result);
  }

  response->set_allocated_value(result);
  response->set_errorcode(memkvErrorToRpcErrno(s.Code()));
  if (!s.IsOK()) {
    response->set_errormessage(s.ToString());
  }
  done->Run();
}

void MemKVServiceImpl::Delete(::google::protobuf::RpcController *controller,
                              const ::memkv::pb::DeleteRequest *request,
                              ::memkv::pb::DeleteResult *response,
                              ::google::protobuf::Closure *done) {}

MemKVServiceImpl::MemKVServiceImpl(DB *db) : db_(db) {}

MemKVServiceImpl::~MemKVServiceImpl() = default;

}  // namespace memkv