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
  PROCESS_EXIT_TYPE,
  COND_ENQUEUE_TYPE,
  COND_INIT_TYPE,
  COND_WAIT_TYPE,
  COND_SIGNAL_TYPE
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

typedef struct cond_init {
  visible_object *cond;
} cond_init;

typedef struct cond_wait {
  visible_object *cond;
  visible_object *mut;
} cond_wait;

typedef struct cond_signal {
  visible_object *cond;
} cond_signal;

typedef struct transition {
  pthread_t executor;
  transition_type type;
  union {
    mutex_init init;
    mutex_lock lock;
    mutex_unlock unlock;
    cond_init cv_init;
    cond_wait cv_wait;
    cond_signal cv_signal;
  };
} transition;

transition invisible_operation_for_this_thread(void);

#ifdef __cplusplus
}
#endif  // extern "C"
