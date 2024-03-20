#include <stdio.h>
#include "mcmini/spy/intercept/interception.h"
#include "mcmini/shared_sem.h"
#include "mcmini/shared_transition.h"
#include "wrappers.h"
#include "mcmini/entry.h"
#include <stdint.h>

typedef uint64_t tid_t;
#define MC_THREAD_LOCAL thread_local
extern MC_THREAD_LOCAL tid_t tid_self;
#define TID_INVALID               (-1ul) // ULONG_MAX
MC_THREAD_LOCAL tid_t tid_self = TID_INVALID;


int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
  printf("Hello from mc_pthread_mutext_init!");
  // TODO: write into the shm region enough information
  // to determine what just happened on the model side

  // The coordinator first assumes data is written as follows:
  // transition id followed by payload.
  //
  // TODO: There's no system in place to synchronize transition ids
  // with the registration on the model side. This is a dynamic process
  // (new transition can be added at runtime)
  // For now, it suffices to assign a fixed value and just assume it
  // corresponds on the model side
    shared_sem_ref cv = &(*trace_list)[tid_self];
    struct MCMutexShadow newlyCreatedMutex;
    initialize_MCMutexShadow(&newlyCreatedMutex, mutex);
    auto newTypeInfo = SharedTransition(-1ul , MutexInit);
    auto newShmData = newlyCreatedMutex;
    extern struct SharedTransition* shmTransitionTypeInfo;
extern void *shmTransitionData;
memcpy(shmTransitionTypeInfo, newTypeInfo, sizeof(SharedTransition));

thread_await_scheduler();
return __real_pthread_mutex_init(mutex, mutexattr);
    }