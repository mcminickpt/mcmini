#pragma once

#include <pthread.h>

#include "mcmini/defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum visible_object_type {
  UNKNOWN,
  MUTEX,
  SEMAPHORE,
  CONDITION_VARIABLE,
  THREAD
} visible_object_type;

typedef enum mutex_state {
  UNINITIALIZED,
  UNLOCKED,
  LOCKED,
  DESTROYED
} mutex_state;

typedef enum thread_status {
  ALIVE,
  EXITED,
} thread_status;

typedef struct semaphore_state {
  int count;
} semaphore_state;

typedef struct thread_state {
  pthread_t pthread_desc;
  runner_id_t id;
  thread_status status;
} thread_state;

// Condition variables ... tricky...

typedef struct visible_object {
  visible_object_type type;
  void *location;
  union {
    mutex_state mutex_state;
    semaphore_state sem_state;
    thread_state thread_state;
  };
} visible_object;

extern visible_object empty_visible_obj;

#ifdef __cplusplus
}
#endif  // extern "C"
