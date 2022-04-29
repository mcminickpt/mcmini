#ifndef GMAL_GMAL_H
#define GMAL_GMAL_H

#include "GMALShared.h"
#include "GMALSharedTransition.h"
#include "GMALDeferred.h"

extern "C" {
    #include <semaphore.h>
    #include "mc_shared_cv.h"
}

/* Synchronization primitives */
extern GMAL_THREAD_LOCAL tid_t tid_self;
extern pid_t cpid;
extern mc_shared_cv (*threadQueue)[MAX_TOTAL_THREADS_IN_PROGRAM];
extern sem_t gmal_pthread_create_binary_sem;

/* Data transfer */
extern void *shmStart;
extern GMALSharedTransition *shmTransitionTypeInfo;
extern void *shmTransitionData;
extern const size_t shmAllocationSize;

/* State */
extern GMALDeferred<GMALState> programState;

GMAL_CTOR void gmal_init();
void gmal_child_panic();

#endif //GMAL_GMAL_H
