#ifndef MC_MC_H
#define MC_MC_H

#include "MCShared.h"
#include "MCSharedTransition.h"
#include "MCDeferred.h"

extern "C" {
    #include <semaphore.h>
    #include "mc_shared_cv.h"
}

/* Synchronization primitives */
extern MC_THREAD_LOCAL tid_t tid_self;
extern pid_t cpid;
extern mc_shared_cv (*threadQueue)[MAX_TOTAL_THREADS_IN_PROGRAM];
extern sem_t mc_pthread_create_binary_sem;

/* Data transfer */
extern void *shmStart;
extern MCSharedTransition *shmTransitionTypeInfo;
extern void *shmTransitionData;
extern const size_t shmAllocationSize;

/* State */
extern MCDeferred<MCState> programState;

MC_CTOR void mc_init();
void mc_child_panic();
void mc_report_undefined_behavior(const char*);

#define MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(x, str) \
do {                                              \
    if (!static_cast<bool>(x)) {                  \
        mc_report_undefined_behavior(static_cast<const char*>(str));      \
    }                                             \
} while(0)

#define MC_REPORT_UNDEFINED_BEHAVIOR(str) MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(false, str)


#endif //MC_MC_H
