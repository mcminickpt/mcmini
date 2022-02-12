#ifndef DPOR_TRANSITION_H
#define DPOR_TRANSITION_H

#include "thread.h"
#include "visible_operation.h"
#include <stdbool.h>

struct transition {
    struct thread thread;
    struct visible_operation operation;
};

typedef struct transition_array transition_array;
typedef struct transition_stack transition_stack;

/// Whether or not the given transition is directly
/// responsible for the creation of the given thread
///
/// \return A nonzero value to signify that the transition
/// is directly responsible for the creation of the given
/// thread, and 0 otherwise
int transition_creates(struct transition, struct thread);

bool transition_enabled(struct transition);

#endif //DPOR_TRANSITION_H
