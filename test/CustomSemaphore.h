#ifndef MCMINI_CUSTOMSEMAPHORE_H
#define MCMINI_CUSTOMSEMAPHORE_H

#include <pthread.h>

typedef struct custom_cond {
    unsigned wakeCount; /* Number of waiters */
    int value; /* Semaphore count */
    pthread_mutex_t mut;
    pthread_cond_t cond;
} custom_sem;

int custom_sem_init(custom_sem*, unsigned, unsigned);
int custom_sem_wait(custom_sem*);
int custom_sem_post(custom_sem*);

#endif //MCMINI_CUSTOMSEMAPHORE_H
