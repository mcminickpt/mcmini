#ifndef MCMINI_CUSTOMCONDITIONVARIABLE_H
#define MCMINI_CUSTOMCONDITIONVARIABLE_H

#include <pthread.h>
#include <semaphore.h>

typedef struct custom_cond {
    int numWaiters;
    pthread_mutex_t internalMut;
    sem_t internalSem;
} custom_cond;

int custom_cond_init(custom_cond*);
int custom_cond_wait(custom_cond*, pthread_mutex_t*);
int custom_cond_signal(custom_cond*);
int custom_cond_broadcast(custom_cond*);


#endif //MCMINI_CUSTOMCONDITIONVARIABLE_H
