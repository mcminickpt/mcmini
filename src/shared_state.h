#ifndef DPOR_SHARED_STATE_H
#define DPOR_SHARED_STATE_H

#include "thread.h"
#include "hashtable.h"
#include "mutex.h"
#include "decl.h"

STRUCT_DECL(shared_state);
struct shared_state {
    mutex_array_ref mutexes;

    // Map threads to their next transition

    // Map threads to transitions (thread to int to transition)
    // Get a hash value from a thread
    // hash_table
    thread_array_ref threads;

    // What each thread does next
    //? transition_mapping;
    hash_table_ref thread_to_transition_mapping;
};
MEMORY_API_DECL(shared_state);


#endif //DPOR_SHARED_STATE_H
