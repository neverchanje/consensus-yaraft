// Copyright 2002 Google Inc.
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

namespace consensus {

// STLDeleteContainerPairSecondPointers()
//  For a range within a container of pairs, calls delete
//  (non-array version) on the SECOND item in the pairs.
// NOTE: Like STLDeleteContainerPointers, deleting behind the iterator.
// Deleting the value does not always invalidate the iterator, but it may
// do so if the key is a pointer into the value object.
// NOTE: If you're calling this on an entire container, you probably want
// to call STLDeleteValues(&container) instead, or use ValueDeleter.
template <class ForwardIterator>
void STLDeleteContainerPairSecondPointers(ForwardIterator begin, ForwardIterator end) {
  while (begin != end) {
    ForwardIterator temp = begin;
    ++begin;
    delete temp->second;
  }
}

// STLDeleteContainerPointers()
//  For a range within a container of pointers, calls delete
//  (non-array version) on these pointers.
// NOTE: for these three functions, we could just implement a DeleteObject
// functor and then call for_each() on the range and functor, but this
// requires us to pull in all of <algorithm>, which seems expensive.
// For hash_[multi]set, it is important that this deletes behind the iterator
// because the hash_set may call the hash function on the iterator when it is
// advanced, which could result in the hash function trying to deference a
// stale pointer.
// NOTE: If you're calling this on an entire container, you probably want
// to call STLDeleteElements(&container) instead, or use an ElementDeleter.
template <class ForwardIterator>
void STLDeleteContainerPointers(ForwardIterator begin, ForwardIterator end) {
  while (begin != end) {
    ForwardIterator temp = begin;
    ++begin;
    delete *temp;
  }
}

}  // namespace consensus