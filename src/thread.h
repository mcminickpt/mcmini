#ifndef DPOR_THREAD_H
#define DPOR_THREAD_H

#include <pthread.h>
#include "array.h"
#include "common.h"
#include "decl.h"

STRUCT_DECL(thread);
struct thread {
    pthread_t owner;
    void *arg;
    thread_routine start_routine;
    volatile int is_alive;
};
typedef array_ref thread_array_ref;
thread_ref thread_create(pthread_t);
thread_ref thread_copy(thread_refc);
void thread_destroy(thread_ref);

STRUCT_DECL(thread_operation);
TYPES_DECL(thread_operation, THREAD_START, THREAD_CREATE, THREAD_JOIN, THREAD_FINISH);
struct thread_operation {
    thread_operation_type type;
    thread_ref thread;
};
MEMORY_API_DECL(thread_operation);

/*
 * Other Memory Functions
 */
thread_ref thread_self(void);
thread_ref thread_wrap(pthread_t);

/*
 * Operations
 */
bool threads_equal(thread_refc, thread_refc);
bool thread_operation_spawns_thread(thread_refc, thread_operation_refc);
bool thread_operation_joins_thread(thread_refc, thread_operation_refc);
bool thread_operation_enabled(thread_operation_refc, thread_refc);

#endif
