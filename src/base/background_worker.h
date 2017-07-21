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

#include <functional>
#include <memory>

#include "base/status.h"

#include <silly/disallow_copying.h>

namespace consensus {

class BackgroundWorker {
  __DISALLOW_COPYING__(BackgroundWorker);

 public:
  BackgroundWorker();

  ~BackgroundWorker();

  // Start looping on the given recurring task infinitely, until users call Stop().
  Status StartLoop(std::function<void()> recurringTask);

  // Stop looping. The function returns error status when the background thread
  // is already stopped.
  Status Stop();

  bool Stopped() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace consensus