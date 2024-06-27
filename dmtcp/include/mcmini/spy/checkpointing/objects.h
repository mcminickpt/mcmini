#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum visible_object_type {
  MUTEX,
  SEMAPHORE,
  CONDITION_VARIABLE,
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

// Condition variables ... tricky...

typedef struct visible_object {
  visible_object_type type;
  void *location;
  union {
    mutex_state mutex_state;
    semaphore_state sem_state;
  };
} visible_object;

#ifdef __cplusplus
}
#endif  // extern "C"
