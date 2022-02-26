#ifndef DPOR_TRANSITION_H
#define DPOR_TRANSITION_H

#include "array.h"
#include "thread.h"
#include "visible_operation.h"
#include <stdbool.h>

STRUCT_DECL(transition);
struct transition {
    thread_ref thread;
    visible_operation_ref operation;
};
typedef array_ref transition_array_ref;
typedef transition_array_ref transition_stack_ref;
PRETTY_PRINT_DECL(transition);

transition_ref transition_alloc(void);
transition_ref transition_create(thread_ref, visible_operation_ref);
transition_ref transition_copy(transition_refc);
void transition_destroy(transition_ref);

/*
 * Operations
 */
transition_ref create_retain_thread_start_transition(thread_ref);
transition_ref create_retain_thread_finish_transition(thread_ref);

thread_ref proc(transition_refc ref);

/**
 * Whether or not the given transition is directly
 * responsible for the creation of the given thread
 *
 * @return A nonzero value to signify that the transition
 * is directly responsible for the creation of the given
 * thread, and 0 otherwise
 */
bool transition_creates(transition_ref, thread_ref);

/**
 * Whether or not the given transition waits until
 * the given thread to exit before becoming enabled
 */
bool transition_waits_on_thread(transition_ref, thread_ref);

/**
 * Determines whether or not the given transition
 * represents a thread in the "exited" state.
 */
bool transition_is_thread_exit(transition_refc);

/**
 * Determines whether or not the provided transition would not
 * block if executed in a real program
 *
 * @return whether or not the provided transition would not
 * block if executed in a real program
 */
bool transition_enabled(transition_ref);

/**
 * Determines whether or not the provided transition would
 * block if executed in a real program
 *
 * @return whether or not the provided transition would
 * block if executed in a real program
 */
inline bool transition_blocked(transition_ref transition)
{
    if (!transition) return false;
    return !transition_enabled(transition);
}

/*
 *
 */
bool transitions_coenabled(transition_ref, transition_ref);

/*
 *
 */
bool transition_happens_before(transition_ref, transition_ref);

/*
 *
 */
inline bool transition_happens_after(transition_ref t1, transition_ref t2)
{
    if (!t1 || !t2) return false;
    return transition_happens_before(t2, t1);
}

/*
 *
 */
bool transitions_dependent(transition_ref, transition_ref);

/*
 *
 */
inline bool transitions_independent(transition_ref t1, transition_ref t2)
{
    return !transitions_dependent(t1, t2);
}

#endif //DPOR_TRANSITION_H
