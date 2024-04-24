#pragma once
#include <stddef.h>

// memcpy implementation but with volatile memory
volatile void *memcpy_v(volatile void *restrict dst,
            const volatile void *restrict src, size_t n);