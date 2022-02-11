#ifndef DPOR_TRANSITION_H
#define DPOR_TRANSITION_H

#include "thread.h"
#include "visible_operation.h"

struct transition {
    struct thread thread;
    struct visible_operation operation;
};

/// Whether or not the given transition is directly
/// responsible for the creation of the given thread
///
/// \return A nonzero value to signify that the transition
/// is directly responsible for the creation of the given
/// thread, and 0 otherwise
int transition_creates(struct transition, struct thread);

#endif //DPOR_TRANSITION_H
