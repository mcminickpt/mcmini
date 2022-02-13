#ifndef DPOR_SHARED_STATE_H
#define DPOR_SHARED_STATE_H

#include "thread.h"
#include "transition.h"
#include "mutex.h"
#include "decl.h"

STRUCT_DECL(shared_state);
struct shared_state {
    mutex_array_ref mutexes;
    thread_array_ref threads;
    transition_array_ref transitions;
};
MEMORY_API_DECL(shared_state);

transition_ref shared_state_first_enabled_transition(shared_state_refc);

#endif //DPOR_SHARED_STATE_H
