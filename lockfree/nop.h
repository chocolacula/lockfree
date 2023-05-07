#pragma once

#include <cstddef>
#if defined(__X86__)
#include <emmintrin.h>
#endif

namespace lockfree {

inline void nop() {
#if defined(__X86__)
  _mm_pause();
#elif defined(__ARM_ARCH)
  asm volatile("yield");
#endif
}

inline void back_off(size_t* n) {
  for (auto i = 0; i < *n; i++) {
    nop();
  }
  *n *= 2;
}

}  // namespace lockfree
