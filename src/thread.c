#include "thread.h"
#include "dpor.h"
#include <stdio.h>

MEMORY_ALLOC_DEF_DECL(thread);
PRETTY_PRINT_DEF_DECL(thread);
MEMORY_ALLOC_DEF_DECL(thread_operation);

thread_ref
thread_create(pthread_t pthread) {
    thread_ref thread = thread_alloc();
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
    if (!other) return NULL;

    thread_ref thread = thread_alloc();
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
    if (!thread) return;
    free(thread);
}

thread_operation_ref
thread_operation_copy(thread_operation_refc ref)
{
    if (!ref) return NULL;
    thread_operation_ref cpy = thread_operation_alloc();
    cpy->thread = thread_copy(ref->thread);
    cpy->type = ref->type;
    return cpy;
}

void
thread_operation_destroy(thread_operation_ref ref)
{
    if (!ref) return;
    thread_destroy(ref->thread);
    free(ref);
}

void
thread_pretty_off(thread_refc t, unsigned int off)
{
    space(off); printf("*** THREAD ***\n");
    space(off); printf("tid: %lu, alive: %d\n", t->tid, t->is_alive);
    space(off); printf("************\n");
}

bool
threads_equal(thread_refc t1, thread_refc t2)
{
    if (t1 == NULL) return t2 == NULL;
    if (t2 == NULL) return t1 == NULL;
    return t1->tid == t2->tid; //pthread_equal(t1->owner, t2->owner);
}

bool thread_operation_spawns_thread(thread_refc t1, thread_operation_refc t2) {
    //TODO Implement function
}

bool thread_operation_joins_thread(thread_refc t1, thread_operation_refc t2) {
    //TODO Implement function
}

bool
thread_operation_enabled(thread_operation_refc top, thread_refc thread)
{
    if (!top || !thread) return false;
    switch (top->type) {
        case THREAD_JOIN:
            return !top->thread->is_alive;
        case THREAD_FINISH:
            return false;
        default:
            return true;
    }
}

hash_t
thread_hash(thread_ref ref)
{
    if (!ref) return 0;
    return ref->tid;
}

inline thread_ref
thread_get_self(void)
{
    return csystem_get_thread_with_tid(&csystem, tid_self);
}

inline thread_ref
thread_get_main_thread(void)
{
    return csystem_get_thread_with_tid(&csystem, TID_MAIN_THREAD);
}