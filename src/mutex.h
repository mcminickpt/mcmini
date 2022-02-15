#ifndef DPOR_MUTEX_H
#define DPOR_MUTEX_H

#include <pthread.h>
#include <stdbool.h>
#include "thread.h"
#include "array.h"
#include "decl.h"

STRUCT_DECL(mutex);
STATES_DECL(mutex, MUTEX_LOCKED, MUTEX_UNLOCKED, MUTEX_UNKNOWN);

STRUCT_DECL(mutex_operation);
TYPES_DECL(mutex_operation, MUTEX_INIT, MUTEX_UNLOCK, MUTEX_DESTROY);

struct mutex {
    mutex_state state;
    thread_ref owner;
    pthread_mutex_t *mutex;
};
typedef array_ref mutex_array_ref;

/*
 * Memory API
 */
mutex_ref mutex_create();
mutex_ref mutex_copy(mutex_ref);
void mutex_destroy(mutex_ref);

/*
 * Operations
 */
bool mutexes_equal(mutex_ref, mutex_ref);
int thread_owns_mutex(thread_ref, mutex_ref);
int mutex_owned(mutex_ref);

// --- MUTEX OPERATION ---
struct mutex_operation {
    mutex_ref mutex;
    enum mutex_operation_type type;
};



/**
 * Determines whether two threads each executing one
 * of the two provided mutex operations could produce
 * a "meaningful" race condition, i.e. one in which
 * the order of their execution matters
 */
bool mutex_operations_race(mutex_operation_refc, mutex_operation_refc);
bool mutex_operation_is_undefined_behavior(mutex_operation_refc);



#endif //DPOR_MUTEX_H
