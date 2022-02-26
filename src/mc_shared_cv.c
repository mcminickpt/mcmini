#include "mc_shared_cv.h"
#include "common.h"

void
mc_shared_cv_init(mc_shared_cv_ref ref)
{
    if (!ref) return;
    sem_init(&ref->dpor_scheduler_sem, SEM_FLAG_SHARED, 0);
    sem_init(&ref->pthread_sem, SEM_FLAG_SHARED, 0);
}

void
mc_shared_cv_destroy(mc_shared_cv_ref ref)
{
    if (!ref) return;
    sem_destroy(&ref->dpor_scheduler_sem);
    sem_destroy(&ref->pthread_sem);
}

void
mc_shared_cv_wait_for_thread(mc_shared_cv_ref ref)
{
    // The scheduler calls this
    sem_wait(&ref->dpor_scheduler_sem);
}

void
mc_shared_cv_wait_for_scheduler(mc_shared_cv_ref ref)
{
    // Threads call this; hence they wait on their own semaphore
    sem_wait(&ref->pthread_sem);
}

void
mc_shared_cv_wake_thread(mc_shared_cv_ref ref)
{
    sem_post(&ref->pthread_sem);
}

void
mc_shared_cv_wake_scheduler(mc_shared_cv_ref ref)
{
    sem_post(&ref->dpor_scheduler_sem);
}