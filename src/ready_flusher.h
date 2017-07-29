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

namespace consensus {

// ReadyFlusher is a single background thread for asynchronously flushing the Ready-s,
// so that the FSM thread can be free from stalls every time when it generates a Ready.

class ReplicatedLogImpl;
class ReadyFlusher {
 public:
  ReadyFlusher();

  ~ReadyFlusher();

  void Register(ReplicatedLogImpl* log);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace consensus
