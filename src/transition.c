#include "transition.h"
#include "common.h"
#include "mutex.h"
#include <stdio.h>

MEMORY_ALLOC_DEF_DECL(transition);
PRETTY_PRINT_DEF_DECL(transition);

void
transition_pretty_off(transition_refc t, unsigned int off)
{
    if (!t) return;
    space(off); printf("****** TRANSITION (addr %p) ******", t);
    space(off); puts("  *************");
    thread_pretty_off(t->thread, off + 2);
    space(off); puts("  *************");
}

transition_ref
transition_create(thread_ref thread, visible_operation_ref visible_op)
{
    if (!thread || !visible_op) return NULL;

    transition_ref ref = transition_alloc();
    if (ref) {
        ref->thread = thread_copy(thread);
        ref->operation = visible_operation_copy(visible_op);
    }
    return ref;
}

transition_ref
transition_copy(transition_refc other)
{
    if (!other) return NULL;
    return transition_create(other->thread, other->operation);
}

void
transition_destroy(transition_ref ref)
{
    if (!ref) return;
    thread_destroy(ref->thread);
    visible_operation_destroy(ref->operation);
    free(ref);
}

static transition_ref
create_retain_thread_transition(thread_ref thread, thread_operation_type type)
{
    thread_operation_ref top = thread_operation_alloc();
    top->type = type;
    top->thread = thread;

    visible_operation_ref vop = visible_operation_alloc();
    vop->type = THREAD_LIFECYCLE;
    vop->thread_operation = top;

    transition_ref trans = transition_alloc();
    trans->operation = vop;
    trans->thread = thread;

    return trans;
}

transition_ref
create_retain_thread_start_transition(thread_ref thread)
{
    return create_retain_thread_transition(thread, THREAD_START);
}

transition_ref
create_retain_thread_finish_transition(thread_ref thread)
{
    return create_retain_thread_transition(thread, THREAD_FINISH);
}

bool
transition_creates(transition_ref tref, thread_ref thread)
{
    if (!tref || !thread) return false;
    return tref->operation->type == THREAD_LIFECYCLE && threads_equal(tref->operation->thread_operation->thread, thread);
}

bool
transition_waits_on_thread(transition_ref tref, thread_ref thread)
{
    if (!tref || !thread) return false;
    if (tref->operation->type != THREAD_LIFECYCLE) return false;
    thread_operation_ref top = visible_operation_unsafely_as_thread_operation(tref->operation);
    if (top->type != THREAD_JOIN) return false;
    return threads_equal(thread, top->thread);
}

bool
transition_is_thread_exit(transition_refc tref)
{
    if (!tref) return false;
    if (visible_operation_is_thread_operation(tref->operation)) {
        thread_operation_ref top = visible_operation_unsafely_as_thread_operation(tref->operation);
        return top->type == THREAD_FINISH;
    }
    return false;
}

bool
transition_enabled(transition_ref transition)
{
    if (!transition) return false;

    if (transition->operation->type == MUTEX) {
        mutex_operation_ref mop = visible_operation_unsafely_as_mutex_operation(transition->operation);
        return mutex_operation_enabled(mop, transition->thread);
    } else if (transition->operation->type == THREAD_LIFECYCLE) {
        thread_operation_ref top = visible_operation_unsafely_as_thread_operation(transition->operation);
        return thread_operation_enabled(top, transition->thread);
    } else {
        return false; // TODO: Add semaphore etc.
    }
}

bool
transitions_coenabled(transition_ref t1, transition_ref t2)
{
    if (!t1 || !t2) return false;

    if (threads_equal(t1->thread, t2->thread))
        return false;

    if (transition_creates(t1, t2->thread) || transition_creates(t2, t1->thread))
        return false;

    if (transition_waits_on_thread(t1, t2->thread) || transition_waits_on_thread(t2, t1->thread))
        return false;

    if (transition_is_thread_exit(t1) || transition_is_thread_exit(t2))
        return false;

    switch (t1->operation->type) {
        case MUTEX:
            if (t2->operation->type != MUTEX) return true;
            mutex_operation_ref vop1 = visible_operation_unsafely_as_mutex_operation(t1->operation);
            mutex_operation_ref vop2 = visible_operation_unsafely_as_mutex_operation(t2->operation);
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
    if (threads_equal(t1->thread, t2->thread))
        return true;

    // t1 creates the thread executing t2 or vice-versa
    if (transition_creates(t1, t2->thread) || transition_creates(t2, t1->thread))
        return true;

    // t1 or t2 is a join operation
    // It is not true that for all states both the thread join
    // and the other operation, whatever it is, are both enabled
    if (t1->operation->type == THREAD_LIFECYCLE
        && t1->operation->thread_operation->type == THREAD_JOIN)
        return true;
    if (t2->operation->type == THREAD_LIFECYCLE
        && t2->operation->thread_operation->type == THREAD_JOIN)
        return true;

    // Mutex operations on the same mutexes
    if (t1->operation->type == MUTEX && t2->operation->type == MUTEX)
        return mutex_operations_race(t1->operation->mutex_operation, t2->operation->mutex_operation);

    return false;
}