#ifndef CV_STATUS_H
#define CV_STATUS_H
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

#endif // CV_STATUS_H
  