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
coop_wait_thread(coop_lock_ref ref)
{
    sem_wait(&ref->pthread_sem);
}

void
coop_wait_scheduler(coop_lock_ref ref)
{
    sem_wait(&ref->dpor_scheduler_sem);
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