#ifndef INCLUDE_MCMINI_MC_SHARED_SEM_HPP
#define INCLUDE_MCMINI_MC_SHARED_SEM_HPP

#include "MCShared.h"
#include <semaphore.h>

// NOTE: We have a semaphore pair for each thread.  trace_sleep_list
//       is an array of mc_shared_sem (no relation to sleep sets).
//  1. The scheduler posts to a pthread_sem for Thread X.
//  2. The scheduler then waits on the dpor_scheduler_sem for Thread X.
//  3. Thread X advances to the next visible operation and then
//       posts to the dpor_scheduler_sem for Thread X.
//  4. Thread X then waits on pthread_sem.
//  5. The scheduler can now decide to post to some new thread, Thread Y.
// NOTE: There is also a separate semaphore for each thread: _create_binary_sem
//       When the scheduler creates a thread, it waits on this semaphore,
//       and the newly created thread then posts on this to the scheduler.
struct mc_shared_sem {
  sem_t dpor_scheduler_sem; // scheduler waits on this; target posts
  sem_t pthread_sem; // target waits on this; scheduler posts
};
typedef struct mc_shared_sem *mc_shared_sem_ref;

void mc_shared_sem_init(mc_shared_sem_ref);
void mc_shared_sem_destroy(mc_shared_sem_ref);

void mc_shared_sem_wait_for_thread(mc_shared_sem_ref);
void mc_shared_sem_wait_for_scheduler(mc_shared_sem_ref);
void mc_shared_sem_wake_thread(mc_shared_sem_ref);
void mc_shared_sem_wake_scheduler(mc_shared_sem_ref);

#endif // INCLUDE_MCMINI_MC_SHARED_SEM_HPP
