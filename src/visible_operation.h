#ifndef DPOR_VISIBLE_OPERATION_H
#define DPOR_VISIBLE_OPERATION_H

#include <pthread.h>
#include "thread.h"
#include "mutex.h"

STRUCT_DECL(visible_operation);
STRUCT_DECL(dynamic_visible_operation);
TYPES_DECL(visible_operation, SEMAPHORE, MUTEX, GLOBAL_ACCESS, THREAD_LIFECYCLE);

struct visible_operation {
    visible_operation_type type;
    union {
        // We need value types here; otherwise we have to
        mutex_operation mutex_operation;
        thread_operation thread_operation;
        // TODO: Extend for semaphores, etc.
    };
};

PRETTY_PRINT_DECL(visible_operation);

struct dynamic_visible_operation {
    visible_operation_type type;
    union {
        dynamic_mutex_operation mutex_operation;
        dynamic_thread_operation thread_operation;
        // TODO: Extend for semaphores, etc.
    };
};

visible_operation dynamic_visible_operation_get_snapshot(dynamic_visible_operation_ref);

/*
 * Operations
 */
bool visible_operation_is_mutex_operation(visible_operation_refc);
bool visible_operation_is_thread_operation(visible_operation_refc);

mutex_operation_ref visible_operation_unsafely_as_mutex_operation(visible_operation_ref);
thread_operation_ref visible_operation_unsafely_as_thread_operation(visible_operation_ref);

#endif //DPOR_VISIBLE_OPERATION_H
