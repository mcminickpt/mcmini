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
MEMORY_API_DECL(transition);

/*
 * Operations
 */
transition_ref create_thread_start_transition(thread_ref);
transition_ref create_thread_finish_transition(thread_ref);


/**
 * Whether or not the given transition is directly
 * responsible for the creation of the given thread
 *
 * \return A nonzero value to signify that the transition
 * is directly responsible for the creation of the given
 * thread, and 0 otherwise
 */
int transition_creates(transition_ref, thread_ref);

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
bool transition_blocked(transition_ref);

bool transitions_coenabled(transition_ref, transition_ref);

bool transition_happens_before(transition_ref, transition_ref);
bool transition_happens_after(transition_ref, transition_ref);
bool transitions_dependent(transition_ref, transition_ref);
bool transitions_independent(transition_ref, transition_ref);

#endif //DPOR_TRANSITION_H
