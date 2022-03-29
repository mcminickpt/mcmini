#include "thread.h"
#include "dpor.h"
#include "fail.h"
#include <stdio.h>

MEMORY_ALLOC_DEF_DECL(thread);
PRETTY_PRINT_DEF_DECL(thread);
PRETTY_PRINT_TYPE_DEF_DECL(thread_operation);
MEMORY_ALLOC_DEF_DECL(thread_operation);

void
thread_pretty_off(thread_refc t, unsigned int off)
{
    space(off); printf("\"THREAD\" tid: %lu, alive: %d\n", t->tid, t->state == THREAD_ALIVE);
}

bool
threads_equal(thread_refc t1, thread_refc t2)
{
    if (t1 == NULL) return t2 == NULL;
    if (t2 == NULL) return t1 == NULL;
    return t1->tid == t2->tid; //pthread_equal(t1->owner, t2->owner);
}

bool
thread_operation_spawns_thread(thread_refc t1, thread_operation_refc t2)
{
    //TODO Implement function
}

bool
thread_operation_joins_thread(thread_refc t1, thread_operation_refc t2)
{
    //TODO Implement function
}

bool
thread_operation_enabled(thread_operation_refc top, thread_refc thread)
{
    if (!top || !thread) return false;

    if (thread->state != THREAD_ALIVE)
        return false;

    switch (top->type) {
        case THREAD_JOIN:
            return top->thread->state != THREAD_ALIVE;
        case THREAD_TERMINATE_PROCESS:
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

void
thread_operation_pretty_off(thread_operation_refc top, unsigned int off)
{
    space(off); printf(" \"THREAD OPERATION\" (address %p)\n", top);
    thread_operation_type_pretty_off(top->type, off + 4);
    thread_pretty_off(top->thread, off + 4);
}

void
thread_operation_type_pretty_off(thread_operation_type state, unsigned int off)
{
    space(off);
    switch (state) {
        case THREAD_START:              puts("THREAD_START"); break;
        case THREAD_FINISH:             puts("THREAD_FINISH"); break;
        case THREAD_JOIN:               puts("THREAD_JOIN"); break;
        case THREAD_CREATE:             puts("THREAD_CREATE"); break;
        case THREAD_TERMINATE_PROCESS:  puts("THREAD_TERMINATE_PROCESS"); break;
        default: mc_unimplemented();
    }
}