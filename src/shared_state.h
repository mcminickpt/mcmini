#ifndef DPOR_SHARED_STATE_H
#define DPOR_SHARED_STATE_H

#include "array.h"
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

transition_ref shared_state_first_enabled_transition_get(shared_state_refc);
transition_ref shared_state_first_enabled_transition_by_thread_get(shared_state_refc, thread_ref);

#endif //DPOR_SHARED_STATE_H
