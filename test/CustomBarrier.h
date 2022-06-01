#ifndef MCMINI_CUSTOMBARRIER_H
#define MCMINI_CUSTOMBARRIER_H

#include <semaphore.h>

/* Based on the Little Book of Semaphores */
typedef struct {
    unsigned n;
    unsigned count;
    pthread_mutex_t mutex;
    sem_t turnstile;
    sem_t turnstile2;
} custom_barrier;

int custom_barrier_init(custom_barrier *, unsigned n);
int custom_barrier_wait(custom_barrier *);

#endif //MCMINI_CUSTOMBARRIER_H
