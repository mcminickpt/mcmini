#include "transition.h"
#include "common.h"
#include "mutex.h"
#include "fail.h"
#include <stdio.h>

MEMORY_ALLOC_DEF_DECL(transition)
PRETTY_PRINT_DEF_DECL(transition)

void
transition_pretty_off(transition_refc t, unsigned int off)
{
    if (!t) return;
    space(off); printf("****** TRANSITION (addr %p) ******\n", t);
    thread_pretty_off(&t->thread, off + 2);
    visible_operation_pretty_off(&t->operation, off + 2);
}

bool
transition_creates(transition_ref tref, thread_ref thread)
{
    if (!tref || !thread) return false;
    return tref->operation.type == THREAD_LIFECYCLE && threads_equal(tref->operation.thread_operation.thread, thread);
}

bool
transition_waits_on_thread(transition_ref tref, thread_ref thread)
{
    if (!tref || !thread) return false;
    if (tref->operation.type != THREAD_LIFECYCLE) return false;
    thread_operation_ref top = visible_operation_unsafely_as_thread_operation(&tref->operation);
    if (top->type != THREAD_JOIN) return false;
    return threads_equal(thread, top->thread);
}

bool
transition_is_thread_exit(transition_refc tref)
{
    if (!tref) return false;
    if (visible_operation_is_thread_operation(&tref->operation)) {
        thread_operation_refc top = &tref->operation.thread_operation;
        return top->type == THREAD_FINISH;
    }
    return false;
}

bool
dynamic_transition_enabled(dynamic_transition_ref dtrans)
{
    // TODO: Prevent copies in the future
    transition t_copy = dynamic_transition_get_snapshot(dtrans);
    return transition_enabled(&t_copy);
}

bool
transition_enabled(transition_ref transition)
{
    if (!transition) return false;

    switch (transition->operation.type) {
        case MUTEX:
            return mutex_operation_enabled(&transition->operation.mutex_operation, &transition->thread);
        case THREAD_LIFECYCLE:
            return thread_operation_enabled(&transition->operation.thread_operation, &transition->thread);
        default:
            mc_unimplemented();
            return false;
    }
}

bool
transitions_coenabled(transition_ref t1, transition_ref t2)
{
    if (!t1 || !t2) return false;

    if (threads_equal(&t1->thread, &t2->thread))
        return false;

    if (transition_creates(t1, &t2->thread) || transition_creates(t2, &t1->thread))
        return false;

    if (transition_waits_on_thread(t1, &t2->thread) || transition_waits_on_thread(t2, &t1->thread))
        return false;

    if (transition_is_thread_exit(t1) || transition_is_thread_exit(t2))
        return false;

    switch (t1->operation.type) {
        case MUTEX:
            if (t2->operation.type != MUTEX) return true;
            mutex_operation_ref vop1 = visible_operation_unsafely_as_mutex_operation(&t1->operation);
            mutex_operation_ref vop2 = visible_operation_unsafely_as_mutex_operation(&t2->operation);
            return mutex_operations_coenabled(vop1, vop2);
        default:
            return true;
    }
}


bool
transitions_dependent(transition_ref t1, transition_ref t2)
{
    if (!t1 || !t2) return false;

    // Execution by the same thread
    if (threads_equal(&t1->thread, &t2->thread))
        return true;

    // t1 creates the thread executing t2 or vice-versa
    if (transition_creates(t1, &t2->thread) || transition_creates(t2, &t1->thread))
        return true;

    // t1 or t2 is a join operation
    // It is not true that for all states both the thread join
    // and the other operation, whatever it is, are both enabled
    if (t1->operation.type == THREAD_LIFECYCLE
        && t1->operation.thread_operation.type == THREAD_JOIN)
        return true;
    if (t2->operation.type == THREAD_LIFECYCLE
        && t2->operation.thread_operation.type == THREAD_JOIN)
        return true;

    // Mutex operations on the same mutexes
    if (t1->operation.type == MUTEX && t2->operation.type == MUTEX)
        return mutex_operations_race(&t1->operation.mutex_operation, &t2->operation.mutex_operation);

    return false;
}

void
dynamic_transition_copy_thread_snapshot(dynamic_thread_operation_ref dtref, thread_operation_ref tref)
{
    tref->type = dtref->type;
    tref->thread = dtref->thread;
}

void
dynamic_transition_copy_mutex_snapshot(dynamic_mutex_operation_ref dmref, mutex_operation_ref mref)
{
    mref->type = dmref->type;
    mref->mutex = *dmref->mutex;
}


transition
dynamic_transition_get_snapshot(dynamic_transition_ref dref)
{
    transition t;
    t.thread = *dref->thread;
    t.operation.type = dref->operation.type;
    switch (dref->operation.type) {
        case MUTEX:
            dynamic_transition_copy_mutex_snapshot(&dref->operation.mutex_operation, &t.operation.mutex_operation);
            break;
        case THREAD_LIFECYCLE:
            dynamic_transition_copy_thread_snapshot(&dref->operation.thread_operation, &t.operation.thread_operation);
            break;
        default:
            mc_unimplemented();
            break;
    }
    return t;
}
