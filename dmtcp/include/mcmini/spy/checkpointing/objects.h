#pragma once

#include <pthread.h>

#include "mcmini/defines.h"
#include "dmtcp/include/mcmini/Thread_queue.h"

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
typedef enum condition_variable_status{
  CV_UNINITIALIZED,
  CV_INITIALIZED,
  CV_WAITING, 
  CV_SIGNALLED,
  CV_TRANSITIONAL
}condition_variable_status;


typedef struct condition_variable_state{
  condition_variable_status status;
  runner_id_t interacting_thread;     // The thread that iscurrently interacting with this condition variable
  pthread_mutex_t *associated_mutex;  // The mutex that is associated with this condition variable
  int count;                    // The number of threads waiting on this condition variable
  thread_queue waiting_threads; // The queue of threads waiting on this condition variable
} condition_variable_state;

typedef struct visible_object {
  visible_object_type type;
  void *location;
  union{
  mutex_state mut_state;
  semaphore_state sem_state;
  condition_variable_state cond_state;
  thread_state thrd_state;
  };
} visible_object;

//extern visible_object empty_object;
#ifdef __cplusplus
}
#endif  // extern "C"
