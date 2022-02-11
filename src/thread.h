#ifndef DPOR_THREAD_H
#define DPOR_THREAD_H

#include <pthread.h>

typedef void*(*thread_routine)(void*);

struct thread {
    pthread_t owner;
    thread_routine start_routine;
    volatile int is_alive;
    void *arg;
};

enum thread_operation_type { THREAD_CREATE, THREAD_JOIN };
struct thread_operation {
    enum thread_operation_type type;
    struct thread thread;
};

int threads_equal(struct thread, struct thread);
int thread_operation_spawns_thread(struct thread, struct thread_operation);
int thread_operation_joins_thread(struct thread, struct thread_operation);

#endif
