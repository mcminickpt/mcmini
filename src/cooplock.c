#include "cooplock.h"
#include "common.h"

void
coop_lock_init(coop_lock_ref ref)
{
    if (!ref) return;
    sem_init(&ref->dpor_scheduler_sem, SEM_FLAG_SHARED, 1);
    sem_init(&ref->pthread_sem, SEM_FLAG_SHARED, 0);
}

void coop_lock_destroy(coop_lock_ref ref)
{
    if (!ref) return;
    sem_destroy(&ref->dpor_scheduler_sem);
    sem_destroy(&ref->pthread_sem);
}

void
coop_wait_for_thread(coop_lock_ref ref)
{
    // The scheduler calls this
    sem_wait(&ref->dpor_scheduler_sem);
}

void
coop_wait_for_scheduler(coop_lock_ref ref)
{
    // Threads call this; hence they wait on their own semaphore
    sem_wait(&ref->pthread_sem);
}

void
coop_wake_thread(coop_lock_ref ref)
{
    sem_post(&ref->pthread_sem);
}

void
coop_wake_scheduler(coop_lock_ref ref)
{
    sem_post(&ref->dpor_scheduler_sem);
}