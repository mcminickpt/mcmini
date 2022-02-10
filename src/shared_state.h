#ifndef DPOR_SHARED_STATE_H
#define DPOR_SHARED_STATE_H

#include "thread.h"
#include "mutex.h"

struct shared_state {
    // All mutexes
    // All threads that are alive at this point

    // Collection of stuff...
    //
};

typedef struct shared_state *shared_state_ref;

shared_state_ref shared_state_create();///


#endif //DPOR_SHARED_STATE_H
