#include <atomic>

namespace lockfree {

struct HazardPtr {

 private:
  thread_local int _ptr[5];
};

struct HazardGuard {};

}  // namespace lockfree
