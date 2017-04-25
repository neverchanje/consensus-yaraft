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

#include "base/status.h"

#include <yaraft/raftpb.pb.h>

namespace consensus {
namespace wal {

using PBEntriesIterator = ::google::protobuf::RepeatedPtrField<::yaraft::pb::Entry>::iterator;
using ConstPBEntriesIterator =
    ::google::protobuf::RepeatedPtrField<const ::yaraft::pb::Entry>::iterator;

class WriteAheadLog {
 public:
  virtual ~WriteAheadLog() = default;

  // Append log entries from message `msg` into underlying storage.
  //
  // Required: msg.type() == pb::MsgApp
  virtual Status AppendEntries(const yaraft::pb::Message& msg) = 0;

  struct CompactionHint {};

  // Do GC work to compact unused logs,
  //
  // On some implementation, GC can be simply done by deleting the unused log files.
  virtual Status GC(CompactionHint* hint) = 0;
};

}  // namespace wal
}  // namespace consensus