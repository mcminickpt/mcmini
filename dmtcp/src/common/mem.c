#include "mcmini/mem.h"

volatile void *memset_v(volatile void *dst, int ch, size_t n) {
  volatile unsigned char *dstc = dst;
  while ((n--) > 0) dstc[n] = ch;
  return dst;
}

volatile void *memcpy_v(volatile void *dst, const volatile void *src,
                        size_t n) {
  // From cppreference on the use of restrict pointers in the C language:
  //
  // | Restricted pointers can be assigned to unrestricted pointers freely, the
  // | optimization opportunities remain in place as long as the compiler is
  // | able to analyze the code:
  // |
  // | void f(int n, float * restrict r, float * restrict s)
  // | {
  // |   float *p = r, *q = s; // OK
  // |    while (n-- > 0)
  // |        *p++ = *q++; // almost certainly optimized just like *r++ = *s++
  // | }
  //
  // See https://en.cppreference.com/w/c/language/restrict for details.
  const volatile unsigned char *srcc = src;
  volatile unsigned char *dstc = dst;
  while ((n--) > 0) dstc[n] = srcc[n];
  return dst;
}
