#include "CustomSemaphore.h"

int
custom_sem_init(custom_sem *sem, unsigned pshared, unsigned value)
{
    /* pshared ignored */
    pthread_mutex_init(&sem->mut, NULL);
    pthread_cond_init(&sem->cond, NULL);
    sem->value = value;
    sem->wakeCount = 0;
    return 0;
}

int
custom_sem_wait(custom_sem *sem)
{
    pthread_mutex_lock(&sem->mut);
    sem->value--;
    if (sem->value < 0) {
        while (sem->wakeCount < 1) {
            pthread_cond_wait(&sem->cond, &sem->mut);
        }
        sem->wakeCount--;
    }
    pthread_mutex_unlock(&sem->mut);
    return 0;
}

int
custom_sem_post(custom_sem *sem)
{
    pthread_mutex_lock(&sem->mut);
    sem->value++;
    if (sem->value <= 0) {
        sem->wakeCount++;
        pthread_cond_signal(&sem->cond);
    }
    pthread_mutex_unlock(&sem->mut);
    return 0;
}
