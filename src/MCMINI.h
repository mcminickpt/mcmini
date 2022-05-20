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
extern sem_t mc_pthread_create_binary_sem;

/* Data transfer */
extern void *shmStart;
extern GMALSharedTransition *shmTransitionTypeInfo;
extern void *shmTransitionData;
extern const size_t shmAllocationSize;

/* State */
extern GMALDeferred<GMALState> programState;

GMAL_CTOR void mc_init();
void mc_child_panic();
void mc_report_undefined_behavior(const char*);

#define GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(x, str) \
do {                                              \
    if (!static_cast<bool>(x)) {                  \
        mc_report_undefined_behavior(static_cast<const char*>(str));      \
    }                                             \
} while(0)

#define GMAL_REPORT_UNDEFINED_BEHAVIOR(str) GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(false, str)


#endif //GMAL_GMAL_H
