#pragma once

#include <pthread.h>

#include "mcmini/defines.h"
#include "mcmini/Thread_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum visible_object_type {
  UNKNOWN,
  MUTEX,
  SEMAPHORE,
  CONDITION_VARIABLE,
  THREAD,
  CV_WAITING_QUEUE
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

typedef struct cv_waiting_queue_state{
  void *cv_location;
  runner_id_t waiting_id;
}cv_waiting_queue_state;

typedef struct thread_state {
  pthread_t pthread_desc;
  runner_id_t id;
  thread_status status;
} thread_state;

/*
  Each state in condition_variable_state corresponds to a phase in the lifescycle
  of a condition variable. The status of a condition variable can be one of the following:
  - CV_UNINITIALIZED: The condition variable has not been initialized yet via mc_pthread_cond_init.
  - CV_INITIALIZED: The condition variable has been initialized and ready for use (post mc_pthread_cond_init).
  - CV_TRANSITIONAL: A thread is releasing the mutex but hasn't fully entered the wait state yet
  (mc_pthread_cond_wait in progress), hence entered outer waiting room. This prevents checkpointing during the
  unsafe gap between mutex unlock and wait.
  - CV_WAITING: The thread has successfully entered the wait state i.e, consumed the signal or successfully returned
  from libpthread_cond_timed_wait. (mc_pthread_cond_wait).
  - CV_SIGNALLED: A signal/broadcast has been sent; the condition variable resumes operation (mc_pthread_cond_signal/broadcast).
  - CV_DESTROYED: The condition variable has been destroyed (post mc_pthread_cond_destroy).
  Note that the status of a condition variable can be CV_SIGNALLED even when there are no threads waiting on it.
  This is because a thread can signal a condition variable even when no threads are waiting on it (spurious wakeup).
                                                  CV_UNINITIALIZED
                                                      │
                                                      │ mc_pthread_cond_init()
                                                      ▼
                                                CV_INITIALIZED ◄───┐
                                                      │            │
                                                      │ mc_pthread_cond_wait() (pre-wait phase)
                                                      ▼            │
                                                CV_TRANSITIONAL    │
                                                      │            │
                                                      │ pthread_cond_timedwait() success
                                                      ▼            │
                                                    CV_WAITING ────┘
                                                      │
                                                      │ mc_pthread_cond_signal()
                                                      ▼
                                                CV_SIGNALLED ──────┐
                                                      ▲            │
                                                      └────────────┘ (thread re-acquires mutex)

                                                CV_DESTROYED (via mc_pthread_cond_destroy)

*/
typedef enum condition_variable_status{
  CV_UNINITIALIZED,
  CV_INITIALIZED,
  CV_WAITING, 
  CV_SIGNALLED,
  CV_TRANSITIONAL,
  CV_DESTROYED
}condition_variable_status;


typedef struct condition_variable_state{
  condition_variable_status status;
  runner_id_t interacting_thread;     // The thread that iscurrently interacting with this condition variable
  pthread_mutex_t *associated_mutex;  // The mutex that is associated with this condition variable
  int count;                    // The number of threads waiting on this condition variable
  thread_queue* waiting_threads; // The queue of threads waiting on this condition variable
} condition_variable_state;

typedef struct visible_object {
  visible_object_type type;
  void *location;
  union{
  mutex_state mut_state;
  semaphore_state sem_state;
  condition_variable_state cond_state;
  thread_state thrd_state;
  cv_waiting_queue_state waiting_queue_state;
  };
} visible_object;

extern visible_object empty_visible_obj;
#ifdef __cplusplus
}
#endif  // extern "C"
