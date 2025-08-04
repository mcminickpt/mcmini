#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// memcpy implementation but with volatile memory
volatile void *memset_v(volatile void *, int ch, size_t n);
volatile void *memcpy_v(volatile void *, const volatile void *, size_t n);

#ifdef __cplusplus
}  // extern "C"
#endif
