#ifndef DPOR_VISIBLE_OPERATION_H
#define DPOR_VISIBLE_OPERATION_H

#include <pthread.h>
#include "thread.h"
#include "mutex.h"

enum visible_operation_type {
    SEMAPHORE, MUTEX, GLOBAL_ACCESS, THREAD_LIFECYCLE
};

struct visible_operation {
    enum visible_operation_type type;
    union {
        struct mutex_operation mutex_operation;
        struct thread_operation thread_operation;
    };
};

int visible_operation_is_mutex_operation(struct visible_operation);
int visible_operation_is_thread_operation(struct visible_operation);


#endif //DPOR_VISIBLE_OPERATION_H
