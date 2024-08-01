#pragma once

#include <pthread.h>

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

typedef struct semaphore_state {
  int count;
} semaphore_state;

typedef struct thread_state {
  pthread_t tag;
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
