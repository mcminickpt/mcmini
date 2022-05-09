// Dining philosophers solution with semaphores

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "GMAL.h"
#include "GMALWrappers.h"

#define NUM_THREADS 5

struct forks {
    int philosopher;
    pthread_mutex_t *left_fork;
    pthread_mutex_t *right_fork;
    sem_t* sem_dining;
} forks[NUM_THREADS];

void * philosopher_doit(void *forks_arg) {
    struct forks *forks = static_cast<struct forks*>(forks_arg);
    gmal_sem_wait(forks->sem_dining);
    gmal_pthread_mutex_lock(forks->left_fork);
    gmal_pthread_mutex_lock(forks->right_fork);

//  printf("Philosopher %d just ate.\n", forks->philosopher);
    gmal_pthread_mutex_unlock(forks->left_fork);
    gmal_pthread_mutex_unlock(forks->right_fork);
    gmal_sem_post(forks->sem_dining);
    return NULL;
}

int main(int argc, char* argv[])
{
    gmal_init();
    pthread_t thread[NUM_THREADS];
    pthread_mutex_t mutex_resource[NUM_THREADS];

    sem_t sem_dining;
    gmal_sem_init(&sem_dining, 0, NUM_THREADS);

    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        // ANSI C/C++ require the cast to pthread_mutex_t, 'struct forks',
        //  respectively, because these are runtime statements, and not declarations
        //    mutex_resource[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        gmal_pthread_mutex_init(&mutex_resource[i], NULL);
        forks[i] = (struct forks){i,
                                  &mutex_resource[i],
                                  &mutex_resource[(i+1) % NUM_THREADS],
                                  &sem_dining};
    }

    for (i = 0; i < NUM_THREADS; i++) {
        gmal_pthread_create(&thread[i], NULL, &philosopher_doit, &forks[i]);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        gmal_pthread_join(thread[i], NULL);
    }

    return 0;
}