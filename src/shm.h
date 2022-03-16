#ifndef DPOR_SHM_H
#define DPOR_SHM_H

#include "decl.h"
#include "visible_operation.h"
#include "transition.h"

STRUCT_DECL(shm_mutex_operation)
struct shm_mutex_operation {
    mutex_operation_type type;
    pthread_mutex_t *mutex;
};

STRUCT_DECL(shm_thread_operation)
struct shm_thread_operation {
    tid_t tid;
    pthread_t target;
    thread_operation_type type;
};

STRUCT_DECL(shm_visible_operation)
struct shm_visible_operation {
    visible_operation_type type;
    union {
        shm_mutex_operation mutex_operation;
        shm_thread_operation thread_operation;
    };
};

STRUCT_DECL(shm_transition)
struct shm_transition {
    thread thread;
    shm_visible_operation operation;
};

#endif //DPOR_SHM_H
