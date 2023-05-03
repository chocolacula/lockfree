#include <atomic>

namespace lockfree {

struct Spin {
  size_t acquire() {
    size_t nop = 0;

    // Wait until the old value is `false`
    bool f = false;
    while (!_state.compare_exchange_weak(  //
        f,                                 // expected
        true,                              // desired
        std::memory_order_relaxed,         //
        std::memory_order_relaxed)) {
          nop++;
    }
    // This fence synchronizes-with store in `release`
    atomic_thread_fence(std::memory_order_acquire);

    return nop;
  }

  void release() {
    _state.store(false, std::memory_order_release);
  }

 private:
  std::atomic_bool _state;
};

struct SpinGuard {
  SpinGuard() {
    auto nop = _spin.acquire();
    std::cout << "the spin released after " << nop << " nops" << std::endl;
  }

  ~SpinGuard() {
    _spin.release();
  }
 private:
  Spin _spin;
};

}  // namespace lockfree
