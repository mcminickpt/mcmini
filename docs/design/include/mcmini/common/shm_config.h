#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "mcmini/defines.h"

const extern size_t shm_size;
void mc_get_shm_handle_name(char *dst, size_t sz);

#ifdef __cplusplus
}
#endif  // extern "C"