#ifndef DPOR_TRANSITION_H
#define DPOR_TRANSITION_H

#include "thread.h"
#include "visible_operation.h"

struct transition {
    struct thread thread;
    struct visible_operation operation;
};

#endif //DPOR_TRANSITION_H
