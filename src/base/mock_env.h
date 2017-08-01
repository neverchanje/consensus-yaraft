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

#include "base/env.h"

namespace consensus {
namespace wal {

class MockWritableFile : public WritableFile {
 public:
  MockWritableFile() {}

  ~MockWritableFile() {}

  Status Append(const Slice& data) override {
    buf_.append(data.data(), data.size());
    return Status::OK();
  }

  Status Truncate(uint64_t size) override {
    buf_.resize(size);
    return Status::OK();
  }

  Status Close() override {
    return Status::OK();
  }

  Status Flush(FlushMode mode) override {
    return Status::OK();
  }

  Status Sync() override {
    return Status::OK();
  }

  uint64_t Size() const override {
    return buf_.size();
  }

  Status PreAllocate(uint64_t size) override {
    return Status::Make(Error::NotSupported);
  }

  const std::string& filename() const override {
    return fileName_;
  }

  std::string Data() const {
    return buf_;
  }

 private:
  std::string buf_;
  std::string fileName_;
};

}  // namespace wal
}  // namespace consensus