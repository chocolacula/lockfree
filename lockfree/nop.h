
#include <emmintrin.h>

namespace lockfree {

inline void nop() {
  _mm_pause();
}

}  // namespace lockfree
