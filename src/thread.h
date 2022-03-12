#ifndef DPOR_THREAD_H
#define DPOR_THREAD_H

#include <pthread.h>
#include "array.h"
#include "hashtable.h"
#include "common.h"
#include "decl.h"

typedef uint64_t tid_t;

STRUCT_DECL(thread);
struct thread {
    tid_t tid;
    //OPEN QUESTION: Shall the owner be the value or reference?
    pthread_t owner;
    void * volatile arg;
    thread_routine start_routine;
    volatile bool is_alive;
};
PRETTY_PRINT_DECL(thread);
typedef array_ref thread_array_ref;


hash_t thread_hash(thread_ref);

thread_ref thread_alloc(void);
thread_ref thread_create(pthread_t);
thread_ref thread_copy(thread_refc);
void thread_destroy(thread_ref);

STRUCT_DECL(thread_operation);
TYPES_DECL(thread_operation, THREAD_START, THREAD_CREATE, THREAD_JOIN, THREAD_FINISH);
struct thread_operation {
    thread_operation_type type;
    csystem_local thread_ref thread;
};
PRETTY_PRINT_DECL(thread_operation);

thread_operation_ref thread_operation_alloc(void);
thread_operation_ref thread_operation_copy(thread_operation_refc);
void thread_operation_destroy(thread_operation_ref);

/*
 * Operations
 */
thread_ref thread_get_self(void);
thread_ref thread_get_main_thread(void);
bool threads_equal(thread_refc, thread_refc);
bool thread_operation_spawns_thread(thread_refc, thread_operation_refc);
bool thread_operation_joins_thread(thread_refc, thread_operation_refc);
bool thread_operation_enabled(thread_operation_refc, thread_refc);

#endif
