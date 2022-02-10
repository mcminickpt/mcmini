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

int threads_equal(struct thread, struct thread);

#endif
