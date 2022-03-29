#ifndef DPOR_MUTEX_H
#define DPOR_MUTEX_H

#include <pthread.h>
#include <stdbool.h>
#include "thread.h"
#include "array.h"
#include "decl.h"

STRUCT_DECL(mutex);
STATES_DECL(mutex, MUTEX_LOCKED, MUTEX_UNLOCKED, MUTEX_UNKNOWN, MUTEX_DESTROYED);

STRUCT_DECL(mutex_operation);
TYPES_DECL(mutex_operation, MUTEX_INIT, MUTEX_LOCK, MUTEX_UNLOCK, MUTEX_DESTROY);
TRANSITION_DESCRIPTION_DECL(mutex_operation)

PRETTY_PRINT_DECL(mutex_operation)
PRETTY_PRINT_TYPE_DECL(mutex_operation)

struct mutex {
    mutid_t mutid;
    mutex_state state;
    csystem_local thread_ref owner;
    pthread_mutex_t *mutex;
};
typedef array_ref mutex_array_ref;

PRETTY_PRINT_DECL(mutex);
PRETTY_PRINT_STATE_DECL(mutex);

hash_t pthread_mutex_hash(pthread_mutex_t*);

/*
 * Operations
 */
bool mutexes_equal(mutex_refc, mutex_refc);
bool dpor_pthread_mutexes_equal(pthread_mutex_t *m1, pthread_mutex_t *m2);
int thread_owns_mutex(thread_ref, mutex_ref);
int mutex_owned(mutex_ref);

// --- MUTEX OPERATION ---
STRUCT_DECL(mutex_operation)
struct mutex_operation {
    mutex mutex;
    enum mutex_operation_type type;
};

STRUCT_DECL(dynamic_mutex_operation)
struct dynamic_mutex_operation {
    csystem_local mutex_ref mutex;
    enum mutex_operation_type type;
};

bool mutex_operation_enabled(mutex_operation_refc, thread_ref);
bool mutex_operations_coenabled(mutex_operation_refc, mutex_operation_refc);
/**
 * Determines whether two threads each executing one
 * of the two provided mutex operations could produce
 * a "meaningful" race condition, i.e. one in which
 * the order of their execution matters
 */
bool mutex_operations_race(mutex_operation_refc, mutex_operation_refc);
bool mutex_operation_creates_mutex(mutex_operation_refc, mutex_refc);
bool mutex_operation_is_undefined_behavior(mutex_operation_refc);



#endif //DPOR_MUTEX_H
