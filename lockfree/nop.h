#pragma once

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

}  // namespace lockfree
