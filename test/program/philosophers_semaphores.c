#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

int DEBUG = 0;

struct forks {
    int philosopher;
    pthread_mutex_t *left_fork;
    pthread_mutex_t *right_fork;
    sem_t* sem_dining;
};

void * philosopher_doit(void *forks_arg) {
    struct forks *forks = forks_arg;
    sem_wait(forks->sem_dining);
    pthread_mutex_lock(forks->left_fork);
    pthread_mutex_lock(forks->right_fork);

    if(DEBUG)
        printf("Philosopher %d is eating.\n", forks->philosopher);
        
    pthread_mutex_unlock(forks->left_fork);
    pthread_mutex_unlock(forks->right_fork);
    sem_post(forks->sem_dining);
    return NULL;
}

int main(int argc, char* argv[])
{
    if(argc != 3){
        printf("Usage: %s NUM_THREADS DEBUG\n", argv[0]);
        return 1;
    }

    int NUM_THREADS = atoi(argv[1]);
    DEBUG = atoi(argv[2]);

    pthread_t thread[NUM_THREADS];
    pthread_mutex_t mutex_resource[NUM_THREADS];
    struct forks forks[NUM_THREADS];

    sem_t sem_dining;
    sem_init(&sem_dining, 0, NUM_THREADS - 1);

    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_mutex_init(&mutex_resource[i], NULL);
        forks[i] = (struct forks){i,
                                  &mutex_resource[i],
                                  &mutex_resource[(i+1) % NUM_THREADS],
                                  &sem_dining};
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&thread[i], NULL, &philosopher_doit, &forks[i]);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread[i], NULL);
    }

    return 0;
}
