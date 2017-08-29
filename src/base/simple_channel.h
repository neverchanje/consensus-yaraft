#pragma once

#include <future>

#include <silly/disallow_copying.h>

namespace consensus {

// SimpleChannel is a golang-like channel implementation that provides inter-thread communication.
//
// On the sender side:
//    auto s = Status::Make(Error::Corruption, "something wrong");
//    SimpleChannel<Status> *ch;
//    (*ch) <<= s;
//
// On the receiver side:
//    Status s;
//    SimpleChannel<Status> *ch;
//    (*ch) >>= s;
//
// SimpleChannel is just a wrapper of the std::future and std::promise.
//

template <typename V>
class SimpleChannel {
  __DISALLOW_COPYING__(SimpleChannel);

 public:
  SimpleChannel() {
    future_ = promise_.get_future();
  }

  SimpleChannel(SimpleChannel &&chan) : promise_(chan.promise_), future_(chan.future_) {}

  void operator<<=(const V &value) {
    promise_.set_value(value);
  }

  void operator<<=(V &&value) {
    promise_.set_value(value);
  }

  void operator>>=(V &value) {
    future_.wait();
    value = future_.get();
  }

 private:
  std::promise<V> promise_;
  std::future<V> future_;
};

class Barrier {
  __DISALLOW_COPYING__(Barrier);

 public:
  Barrier() {
    future_ = promise_.get_future();
  }

  void Signal() {
    promise_.set_value();
  }

  void Wait() {
    future_.wait();
  }

 private:
  std::promise<void> promise_;
  std::future<void> future_;
};

}  // namespace consensus