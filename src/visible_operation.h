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
        mutex_operation mutex_operation;
        thread_operation thread_operation;
    };
};

visible_operation_ref visible_operation_alloc(void);
visible_operation_ref visible_operation_copy(visible_operation_refc);
void visible_operation_destroy(visible_operation_ref);

/*
 * Operations
 */
bool visible_operation_is_mutex_operation(visible_operation_refc);
bool visible_operation_is_thread_operation(visible_operation_refc);

mutex_operation_ref visible_operation_unsafely_as_mutex_operation(visible_operation_ref);
thread_operation_ref visible_operation_unsafely_as_thread_operation(visible_operation_ref);

#endif //DPOR_VISIBLE_OPERATION_H
