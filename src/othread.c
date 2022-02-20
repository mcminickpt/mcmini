#include "othread.h"

MEMORY_ALLOC_DEF_DECL(othread)

othread_ref
othread_create(thread_ref thr)
{
    othread_ref thread = othread_alloc();
    if (!thread) return NULL;
    thread->thread = thread_copy(thr);
    sem_init(&thread->pthread_sem, 0, 0);
    sem_init(&thread->dpor_scheduler_sem, 0, 1);
    return thread;
}

othread_ref
othread_copy(othread_refc other)
{
    return NULL;
}

void
othread_destroy(othread_ref othread)
{
    if (!othread) return;
    thread_destroy(othread->thread);
    sem_destroy(&othread->pthread_sem);
    sem_destroy(&othread->dpor_scheduler_sem);
    free(othread);
}

othread_ref
othread_pthread_wrap(pthread_t pthread)
{
    thread_ref thread = thread_create(pthread);
    return othread_create(thread);
}