#include <atomic>
#include <cstddef>
#include <iostream>

#include "nop.h"

namespace lockfree {

struct Spin {
  size_t acquire() {
    size_t n = 0;
    size_t b = 1;

    // Wait until the old value is `false`
    bool f = false;
    while (!_state.compare_exchange_weak(  //
        f,                                 // expected
        true,                              // desired
        std::memory_order_relaxed,         //
        std::memory_order_relaxed)) {
      // use exponential back off
      n = b;
      back_off(&b);
    }
    // This fence synchronizes-with store in `release`
    atomic_thread_fence(std::memory_order_acquire);

    return n;
  }

  void release() {
    _state.store(false, std::memory_order_release);
  }

 private:
  std::atomic_bool _state;
};

struct SpinGuard {
  SpinGuard() {
    auto n = _spin.acquire();
    std::cout << "the spin released after " << n << " nops" << std::endl;
  }

  ~SpinGuard() {
    _spin.release();
  }

 private:
  Spin _spin;
};

}  // namespace lockfree
