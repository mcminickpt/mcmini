// Dining philosophers solution with semaphores

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "../CustomSemaphore.h"

int DEBUG = 0;

struct forks {
    int philosopher;
    pthread_mutex_t *left_fork;
    pthread_mutex_t *right_fork;
} *forks;

custom_sem sem_dining;

void * philosopher_doit(void *forks_arg) {
    struct forks *forks = forks_arg;
    custom_sem_wait(&sem_dining);
    pthread_mutex_lock(forks->left_fork);
    pthread_mutex_lock(forks->right_fork);

    if(DEBUG) printf("Philosopher %d is eating\n", forks->philosopher);

    pthread_mutex_unlock(forks->left_fork);
    pthread_mutex_unlock(forks->right_fork);
    custom_sem_post(&sem_dining);
    return NULL;
}

int main(int argc, char* argv[]) {
    if(argc != 3){
        printf("Usage: %s NUM_PHILOSOPHERS DEBUG\n", argv[0]);
        return 1;
    }

    int NUM_THREADS = atoi(argv[1]);
    DEBUG = atoi(argv[2]);
    pthread_t thread[NUM_THREADS];
    pthread_mutex_t mutex_resource[NUM_THREADS];
    forks = malloc(NUM_THREADS * sizeof(struct forks));

    custom_sem_init(&sem_dining, 0, NUM_THREADS);

    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_mutex_init(&mutex_resource[i], NULL);
        forks[i] = (struct forks){i,
                                  &mutex_resource[i],
                                  &mutex_resource[(i+1) % NUM_THREADS]};
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&thread[i], NULL, &philosopher_doit, &forks[i]);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread[i], NULL);
    }

    free(forks);
    return 0;
}
