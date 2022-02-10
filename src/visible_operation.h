#ifndef DPOR_VISIBLE_OPERATION_H
#define DPOR_VISIBLE_OPERATION_H

#include <pthread.h>
#include "thread.h"
#include "mutex.h"

enum visible_operation_type {
    SEMAPHORE, MUTEX, GLOBAL_ACCESS, THREAD_SPAWN
};

enum mutex_operation_type { MUTEX_INIT, MUTEX_LOCK, MUTEX_UNLOCK, MUTEX_DESTROY };
struct mutex_operation {
    struct mutex mutex;
    enum mutex_operation_type type;
};

enum thread_spawn_operation_type { THREAD_CREATE, THREAD_JOIN };
struct thread_spawn_operation {
    enum thread_spawn_operation_type type;
    struct thread thread;
};

struct visible_operation {
    enum visible_operation_type type;
    union {
        struct mutex_operation mutex_operation;
        struct thread_spawn_operation thread_spawn_operation;
    };
};

#endif //DPOR_VISIBLE_OPERATION_H
