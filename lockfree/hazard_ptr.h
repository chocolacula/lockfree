#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

namespace lockfree {

class Hazard {
 public:
  struct Ptr {
    Ptr() : _ptr(nullptr), _del(nullptr) {
    }

    // T *p, void (*del)(void* p) = &Ptr::del<T>

    template <typename T>
    Ptr& operator=(T* p) {
      _ptr = p;
      _del = &del<T>;

      return *this;
    }

    void clear() {
      _ptr = nullptr;
      _del = nullptr;
    }

    template <typename T>
    static void del(void* p) {
      delete static_cast<T*>(p);
    }

   private:
    friend class Hazard;

    void drop() {
      _del(_ptr);
      clear();
    }

    void* _ptr;
    void (*_del)(void* p);
  };

  static Ptr* get(size_t i) {
    auto& hp = _pointers[thread_id()];
    return &hp[i];
  }

  static void retire(Ptr* p) {
    _rlist[_rsize++] = *p;
    p->clear();

    if (_rsize == R) {
      scan();
    }
  }

  static void scan() {
    auto tid = thread_id();

    size_t d_size = 0;
    void* dlist[(P - 1) * K];

    for (auto j = 0; j < P; j++) {
      for (auto i = 0; i < K; i++) {

        // get all non free pointers from all descriptors except current one
        if (j == tid) {
          continue;
        }

        auto hz = _pointers[j][i];
        if (hz._ptr != nullptr) {
          dlist[d_size++] = hz._ptr;
        }
      }
    }

    std::sort(dlist, dlist + d_size,  // contain
              [](void* a, void* b) { return a > b; });

    // check if pointers in current descriptor are used by other threads
    auto size = _rsize;
    for (auto i = 0; i < size; i++) {
      auto& p = _rlist[i];

      // if the ptr is not used by other threads, drop it
      if (!std::binary_search(dlist, dlist + d_size, p._ptr)) {
        p.drop();
        _rsize--;
      }
    }
    std::remove_if(_rlist, _rlist + _rsize,  //
                   [](const Ptr& p) { return p._ptr == nullptr; });
  }

 private:
  static const size_t P = 10;  // threads number
  static const size_t K = 4;   // num of hazard pointer in a thread
  static const size_t R = K;   // size of buffer for retired pinter

  static inline Ptr _pointers[P][K];

  static inline thread_local size_t _rsize = 0;
  static inline thread_local Ptr _rlist[K * 4];

  static size_t thread_id() {
    static std::atomic_size_t th_id = 0;

    static thread_local size_t id = th_id++;
    return id;
  }
};

struct HazardGuard {
  template <typename T>
  explicit HazardGuard(size_t i) {
  }

  ~HazardGuard() {
    Hazard::retire(_ptr);
  }

 private:
  Hazard::Ptr* _ptr;
};

}  // namespace lockfree
