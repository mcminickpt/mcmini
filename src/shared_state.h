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

transition_ref shared_state_get_first_enabled_transition(shared_state_refc);
transition_ref shared_state_get_first_enabled_transition_by_thread(shared_state_refc, thread_ref);
transition_array_ref shared_state_create_enabled_transitions(shared_state_refc);

bool shared_state_has_mutex(mutex_ref);

shared_state_ref next(shared_state_ref, transition_ref, transition_ref);

#endif //DPOR_SHARED_STATE_H
