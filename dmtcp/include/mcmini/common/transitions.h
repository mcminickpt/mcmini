#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "mcmini/defines.h"

typedef enum {
  MUTEX_INIT_TYPE,
  MUTEX_LOCK_TYPE,
  MUTEX_UNLOCK_TYPE,
  THREAD_CREATE_TYPE,
  THREAD_JOIN_TYPE,
  THREAD_EXIT_TYPE,
  PROCESS_EXIT_TYPE
  // sem_init,
  // sem_wait,
  // sem_post,
  // thread_create,
  // thread_join
  //...
} transition_type;

#ifdef __cplusplus
}
#endif  // extern "C"
