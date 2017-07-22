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

#include <map>
#include <set>

#include "base/logging.h"

#include "wal_commit_observer.h"

namespace consensus {

class WalCommitObserver::Impl {
 public:
  void Register(std::pair<uint64_t, uint64_t> range, SimpleChannel<Status> *channel) {
    std::lock_guard<std::mutex> g(mu_);

    WalWritesMap::const_iterator it = writes_.find(range);
    if (it != writes_.end()) {
      FMT_LOG(ERROR,
              "WalCommitObserver::Register: duplicate calls for a range of entries in [{}, {}]",
              range.first, range.second);
    } else {
      writes_[range] = channel;
    }
  }

  void Notify(uint64_t commitIndex) {
    std::lock_guard<std::mutex> g(mu_);

    auto it = writes_.begin();
    for (; it != writes_.end(); it++) {
      if (commitIndex >= it->first.second) {
        (*it->second) <<= Status::OK();
      }
      writes_.erase(it);
    }
  }

 private:
  typedef std::map<std::pair<uint64_t, uint64_t>, SimpleChannel<Status> *> WalWritesMap;
  WalWritesMap writes_;

  std::mutex mu_;
};

void WalCommitObserver::Register(std::pair<uint64_t, uint64_t> range,
                                 SimpleChannel<Status> *channel) {
  impl_->Register(range, channel);
}

void WalCommitObserver::Notify(uint64_t commitIndex) {
  impl_->Notify(commitIndex);
}

WalCommitObserver::~WalCommitObserver() = default;

WalCommitObserver::WalCommitObserver() : impl_(new Impl) {}

}  // namespace consensus