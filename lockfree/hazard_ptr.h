#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace lockfree {

class Hazard {
 public:
  struct Ptr {
    Ptr() : _ptr(nullptr) {
    }

    void* get() const {
      return _ptr;
    }

    void set(void* ptr) {
      _ptr = ptr;
    }

    template <typename T>
    Ptr& operator=(T* p) {
      _ptr = p;

      return *this;
    }

    bool operator==(const Ptr& other) const {
      return _ptr == other._ptr;
    }

    bool operator!=(const Ptr& other) const {
      return !(_ptr == other._ptr);
    }

    bool operator==(void* other) const {
      return _ptr == other;
    }

    bool operator!=(void* other) const {
      return !(_ptr == other);
    }

    void clear() {
      _ptr = nullptr;
    }

   private:
    void* _ptr;
  };

  static Ptr* get(size_t i) {
    return &_pointers[thread_id()][i];
  }

  template <typename T>
  static void retire(T* p, void (*del)(void* p) = &Hazard::del<T>) {
    _rlist[_rsize++] = p;

    if (_rsize == R) {
      scan(del);
    }
  }

  static void scan(void (*del)(void* p)) {
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
        if (hz != nullptr) {
          dlist[d_size++] = hz.get();
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
      if (!std::binary_search(dlist, dlist + d_size, p)) {
        del(p);
        p = nullptr;
        _rsize--;
      }
    }
    std::remove_if(_rlist, _rlist + _rsize,  //
                   [](const void* p) { return p == nullptr; });
  }

  template <typename T>
  static void del(void* p) {
    delete static_cast<T*>(p);
  }

 private:
  static const size_t P = 100;    // threads number
  static const size_t K = 4;      // num of hazard pointer in a thread
  static const size_t R = K * 2;  // size of buffer for retired pinter

  static inline Ptr _pointers[P][K];

  static inline thread_local size_t _rsize = 0;
  static inline thread_local void* _rlist[R];

  static size_t thread_id() {
    static std::atomic_size_t th_id = 0;

    static thread_local size_t id = th_id++;

    if (id > P) {
      std::string s = "thread limit exceeded, thread id = ";
      s += std::to_string(id);
      throw std::runtime_error(s);
    }
    return id;
  }
};

template <typename T>
struct HazardGuard {
  HazardGuard(size_t i, T* p, void (*del)(void* p) = &Hazard::del<T>) : _ptr(p), _del(del) {
    Hazard::get(i)->set(p);
  }

  ~HazardGuard() {
    Hazard::retire(_ptr, _del);
  }

 private:
  T* _ptr;
  void (*_del)(void* p);
};

}  // namespace lockfree
