#ifndef DPOR_TRANSITION_H
#define DPOR_TRANSITION_H

#include "thread.h"
#include "visible_operation.h"
#include <stdbool.h>

STRUCT_DECL(transition);
struct transition {
    thread_ref thread;
    visible_operation_ref operation;
};
typedef struct transition_array transition_array;
typedef struct transition_stack transition_stack;
MEMORY_API_DECL(transition);

/*
 * Operations
 */

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

#endif //DPOR_TRANSITION_H
