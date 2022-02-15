#include "thread.h"

thread_ref
thread_self(void)
{
    return thread_create(pthread_self());
}

thread_ref
thread_create(pthread_t pthread) {
    thread_ref thread = malloc(sizeof(*thread));
    if (!thread) return NULL;
    thread->is_alive = true;
    thread->start_routine = NULL;
    thread->arg = NULL;
    thread->owner = pthread;
    return thread; // owner unspecified
}

thread_ref
thread_copy(thread_refc other)
{
    thread_ref thread = malloc(sizeof(*thread));
    if (!thread) return NULL;
    thread->is_alive = other->is_alive;
    thread->start_routine = other->start_routine;
    thread->arg = other->arg;
    thread->owner = other->owner;
    return thread;
}

void
thread_destroy(thread_ref thread)
{

}

bool
threads_equal(thread_refc t1, thread_refc t2)
{
    if (t1 == NULL) return t2 == NULL;
    if (t2 == NULL) return t1 == NULL;
    return pthread_equal(t1->owner, t2->owner);
}