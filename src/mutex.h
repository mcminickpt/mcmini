#ifndef DPOR_MUTEX_H
#define DPOR_MUTEX_H

#include <pthread.h>
#include "thread.h"

enum mutex_state { MUTEX_LOCKED, MUTEX_UNLOCKED, MUTEX_UNKNOWN };
struct mutex {
    enum mutex_state state;
    pthread_mutex_t *mutex;
    union {
        struct thread owner; // Only defined if the lock is currently held
    };
};

int mutexes_equal(struct mutex, struct mutex);

enum mutex_operation_type { MUTEX_INIT, MUTEX_LOCK, MUTEX_UNLOCK, MUTEX_DESTROY };
struct mutex_operation {
    struct mutex mutex;
    enum mutex_operation_type type;
};

typedef struct mutex_array mutex_array;

/// Determines whether two threads each executing one
/// of the two provided mutex operations could produce
/// a "meaningful" race condition, i.e. one in which
/// the order of their execution matters
int mutex_operations_race(struct mutex_operation, struct mutex_operation);
int mutex_operation_is_undefined_behavior(struct mutex_operation);


int thread_owns_mutex(struct thread, struct mutex);
int mutex_owned(struct mutex);


#endif //DPOR_MUTEX_H
