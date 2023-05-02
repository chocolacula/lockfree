#include <atomic>

namespace lockfree {

struct Spinlock {
  void lock() {
    // Wait until the old value is `false`
    bool f = false;
    while (!_state.compare_exchange_weak(  //
        f,                                 // expected
        true,                              // desired
        std::memory_order_relaxed,         //
        std::memory_order_relaxed)) {
    }
    // This fence synchronizes-with store in `unlock`
    atomic_thread_fence(std::memory_order_acquire);
  }

  void unlock() {
    _state.store(false, std::memory_order_release);
  }

 private:
  std::atomic_bool _state;
};

}  // namespace lockfree
