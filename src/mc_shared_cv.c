#include "mc_shared_cv.h"
#include "common.h"
#include <stdio.h>
#include <pthread.h>
#include "dpor.h"

PRETTY_PRINT_DEF_DECL(mc_shared_cv)

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
//    puts("** SCHEDULER **");
//    printf("Wait for thread on %p %lu\n", &ref->dpor_scheduler_sem, pthread_self());
    sem_wait(&ref->dpor_scheduler_sem);
//    puts("Wait for thread over");
//    puts("*********");
}

void
mc_shared_cv_wait_for_scheduler(mc_shared_cv_ref ref)
{
    // Threads call this; hence they wait on their own semaphore
//    puts("** THREAD IN PROGRAM **");
//    printf("Wait for scheduler on %p %lu %lu\n", &ref->pthread_sem, pthread_self(), tid_self);
    sem_wait(&ref->pthread_sem);
//    puts("Wait for scheduler over");
//    puts("*********");
}

void
mc_shared_cv_wake_thread(mc_shared_cv_ref ref)
{
//    puts("** SCHEDULER **");
//    printf("Wake thread on %p %lu\n", &ref->pthread_sem, pthread_self());
    sem_post(&ref->pthread_sem);
//    puts("Wake thread over");
//    puts("*********");
}

void
mc_shared_cv_wake_scheduler(mc_shared_cv_ref ref)
{
//    puts("** THREAD IN PROGRAM **");
//    printf("Wake scheduler on %p %lu %lu\n", &ref->dpor_scheduler_sem, pthread_self(), tid_self);
    sem_post(&ref->dpor_scheduler_sem);
//    puts("Wake scheduler over");
//    puts("*********");
}

void
mc_shared_cv_pretty_off(mc_shared_cv_refc ref, unsigned int off)
{
    space(off);
    int dpor_sem_value, pthread_sem_value;
    sem_getvalue(&ref->pthread_sem, &pthread_sem_value);
    sem_getvalue(&ref->dpor_scheduler_sem, &dpor_sem_value);
    printf("dpor_scheduler_sem count: %d, pthread_count: %d\n", dpor_sem_value, pthread_sem_value);
}