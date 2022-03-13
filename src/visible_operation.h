#ifndef DPOR_VISIBLE_OPERATION_H
#define DPOR_VISIBLE_OPERATION_H

#include <pthread.h>
#include "thread.h"
#include "mutex.h"

STRUCT_DECL(visible_operation);
TYPES_DECL(visible_operation, SEMAPHORE, MUTEX, GLOBAL_ACCESS, THREAD_LIFECYCLE);

struct visible_operation {
    visible_operation_type type;
    union {
        // We need value types here; otherwise we have to
        // malloc for each transition in the transition stack.
        mutex_operation mutex_operation;
        thread_operation thread_operation;
        // TODO: Extend for semaphores, etc.
    };
};

/*
 * Operations
 */
bool visible_operation_is_mutex_operation(visible_operation_refc);
bool visible_operation_is_thread_operation(visible_operation_refc);

mutex_operation_ref visible_operation_unsafely_as_mutex_operation(visible_operation_ref);
thread_operation_ref visible_operation_unsafely_as_thread_operation(visible_operation_ref);

#endif //DPOR_VISIBLE_OPERATION_H
