#pragma once
#include <stddef.h>

// memcpy implementation but with volatile memory
volatile void *memset_v(volatile void *restrict dst, int ch, size_t n);
volatile void *memcpy_v(volatile void *restrict dst,
            const volatile void *restrict src, size_t n);