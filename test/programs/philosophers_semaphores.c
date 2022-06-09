// Dining philosophers solution with semaphores

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 4

struct forks {
    int philosopher;
    pthread_mutex_t *left_fork;
    pthread_mutex_t *right_fork;
    sem_t* sem_dining;
} forks[NUM_THREADS];

void * philosopher_doit(void *forks_arg) {
    struct forks *forks = forks_arg;
    sem_wait(forks->sem_dining);
    pthread_mutex_lock(forks->left_fork);
    pthread_mutex_lock(forks->right_fork);

//  printf("Philosopher %d just ate.\n", forks->philosopher);
    pthread_mutex_unlock(forks->left_fork);
    pthread_mutex_unlock(forks->right_fork);
    sem_post(forks->sem_dining);
    return NULL;
}

int main(int argc, char* argv[])
{
    pthread_t thread[NUM_THREADS];
    pthread_mutex_t mutex_resource[NUM_THREADS];

    sem_t sem_dining;
    sem_init(&sem_dining, 0, NUM_THREADS - 1);

    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        // ANSI C++ require the cast to pthread_mutex_t, 'struct forks',
        //  respectively, because these are runtime statements,
        //  and not declarations.  But not in C.
        //    mutex_resource[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
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
