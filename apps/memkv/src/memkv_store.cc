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
#include <unordered_map>

#include "logging.h"
#include "memkv_store.h"

#include <boost/algorithm/string/split.hpp>

namespace boost {
template <>
struct range_iterator<memkv::Slice> {
  typedef memkv::Slice::const_iterator type;
};
}  // namespace boost

namespace memkv {

struct Node {
  using ChildrenTable = std::unordered_map<std::string, Node *>;
  ChildrenTable children;
  std::string data;
};

class MemKvStore::Impl {
 public:
  Status Write(const Slice &path, const Slice &value) {
    std::vector<Slice> pathVec;
    ASSIGN_IF_OK(validatePath(path), pathVec);

    Node *n = &root_;
    Node *parent = n;
    for (const Slice &seg : pathVec) {
      // ignore empty segment
      if (seg.Len() == 0) {
        continue;
      }

      auto result = n->children.emplace(std::make_pair(seg.ToString(), nullptr));
      bool nodeNotExist = result.second;
      Node **pNode = &result.first->second;
      if (nodeNotExist) {
        *pNode = new Node();
      }

      parent = n;
      n = *pNode;
    }

    parent->data = value.ToString();
    return Status::OK();
  }

  Status Delete(const Slice &path) {
    std::vector<Slice> pathVec;
    ASSIGN_IF_OK(validatePath(path), pathVec);

    Node *n = &root_;
    Node *parent = n;
    for (auto segIt = pathVec.begin(); segIt != pathVec.end(); segIt++) {
      Slice seg = *segIt;

      if (seg.Len() == 0) {
        continue;
      }

      auto it = n->children.find(seg.ToString());
      if (it == n->children.end()) {
        // the given path is deleted
        return Status::OK();
      }

      // we have found the matched node, delete it now.
      if (std::next(segIt) == pathVec.end()) {
        n->children.erase(it);
        return Status::OK();
      }

      parent = n;
      n = it->second;
    }

    return Status::Make(Error::InvalidArgument, "cannot delete root directory");
  }

  Status Get(const Slice &path, std::string *data) {
    std::vector<Slice> pathVec;
    ASSIGN_IF_OK(validatePath(path), pathVec);

    Node *n = &root_;
    Node *parent = n;
    for (const Slice &seg : pathVec) {
      if (seg.Len() == 0) {
        continue;
      }

      auto it = n->children.find(seg.ToString());
      if (it == n->children.end()) {
        return FMT_Status(Error::NodeNotExist, "node does not exist on path {}", path.data());
      }

      parent = n;
      n = it->second;
    }

    *data = parent->data;
    return Status::OK();
  }

 private:
  StatusWith<std::vector<Slice>> validatePath(const Slice &p) {
    Slice path = p;
    path.TrimSpace();
    if (UNLIKELY(path.Len() == 0)) {
      return Status::Make(Error::InvalidArgument, "path is empty");
    }

    for (size_t i = 0; i < path.size(); i++) {
      if (path[i] == '\0') {
        return FMT_Status(Error::InvalidArgument, "path contains NUL at index {}", i);
      }
    }

    std::vector<Slice> result;
    boost::split(result, path, [](char c) { return c == '/'; });
    return result;
  }

 private:
  Node root_;

  mutable std::mutex mu_;
};

Status MemKvStore::Write(const Slice &path, const Slice &value) {
  return impl_->Write(path, value);
}

Status MemKvStore::Delete(const Slice &path) {
  return impl_->Delete(path);
}

Status MemKvStore::Get(const Slice &path, std::string *data) {
  return impl_->Get(path, data);
}

MemKvStore::MemKvStore() : impl_(new Impl) {}

MemKvStore::~MemKvStore() = default;

}  // namespace memkv