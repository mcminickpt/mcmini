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
        mutex_operation_ref mutex_operation;
        thread_operation_ref thread_operation;
    };
};

/*
 * Memory API
 */
MEMORY_API_DECL(visible_operation);

/*
 * Operations
 */
int visible_operation_is_mutex_operation(visible_operation_ref);
int visible_operation_is_thread_operation(visible_operation_ref);

mutex_operation_ref visible_operation_unsafely_as_mutex_operation(visible_operation_ref);
thread_operation_ref visible_operation_unsafely_as_thread_operation(visible_operation_ref);

#endif //DPOR_VISIBLE_OPERATION_H
