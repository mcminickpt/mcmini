#include "CustomBarrier.h"
#include <pthread.h>

// I'm not entirely confident that this implementation
// will work according to the semantics of a barrier
// with N threads if there are more than N threads
// interacting with the barrier
static int
custom_barrier_wait_turnstile1(custom_barrier *barrier)
{
    pthread_mutex_lock(&barrier->mutex);
    if (++barrier->count == barrier->n) {
        for (unsigned i = 0; i < barrier->n; i++)
            sem_post(&barrier->turnstile);
    }
    pthread_mutex_unlock(&barrier->mutex);
    sem_wait(&barrier->turnstile);
    return 0;
}

static int
custom_barrier_wait_turnstile2(custom_barrier *barrier)
{
    pthread_mutex_lock(&barrier->mutex);
    if (--barrier->count == 0) {
        for (unsigned i = 0; i < barrier->n; i++)
            sem_post(&barrier->turnstile2);
    }
    pthread_mutex_unlock(&barrier->mutex);
    sem_wait(&barrier->turnstile2);
    return 0;
}

int
custom_barrier_init(custom_barrier *barrier, unsigned n)
{
    barrier->count = 0;
    barrier->n = n;
    pthread_mutex_init(&barrier->mutex, NULL);
    sem_init(&barrier->turnstile, 0, 0);
    sem_init(&barrier->turnstile2, 0, 0);
    return 0;
}

int
custom_barrier_wait(custom_barrier *barrier)
{
    custom_barrier_wait_turnstile1(barrier);
    custom_barrier_wait_turnstile2(barrier);
}