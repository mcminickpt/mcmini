#ifndef DPOR_SHARED_STATE_H
#define DPOR_SHARED_STATE_H

#include "thread.h"
#include "mutex.h"

struct shared_state {
    mutex_array mutexes;

    // Map threads to their next transition

    // Map threads to transitions (thread to int to transition)
    // Get a hash value from a thread
    // hash_table
    thread_array threads;

    // What each thread does next
    //? transition_mapping;
};

//typedef struct shared_state *shared_state_ref;
//shared_state_ref shared_state_create();///

#endif //DPOR_SHARED_STATE_H
