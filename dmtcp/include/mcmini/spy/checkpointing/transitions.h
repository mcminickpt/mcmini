#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>

#include "mcmini/defines.h"
#include "mcmini/spy/checkpointing/objects.h"

typedef enum transition_type {
  INVISIBLE_OPERATION_TYPE,
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

typedef struct mutex_init {
  visible_object *mut;
} mutex_init;

typedef struct mutex_lock {
  visible_object *mut;
} mutex_lock;

typedef struct mutex_unlock {
  visible_object *mut;
} mutex_unlock;

typedef struct transition {
  pthread_t executor;
  transition_type type;
  union {
    mutex_init init;
    mutex_lock lock;
    mutex_unlock unlock;
  };
} transition;

transition invisible_operation_for_this_thread(void);

#ifdef __cplusplus
}
#endif  // extern "C"
