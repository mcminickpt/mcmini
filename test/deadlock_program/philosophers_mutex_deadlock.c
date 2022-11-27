// Naive dining philosophers solution, which leads to deadlock.

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 9

struct forks {
    int philosopher;
    pthread_mutex_t *left_fork;
    pthread_mutex_t *right_fork;
} forks[NUM_THREADS];

void * philosopher_doit(void *forks_arg) {
    struct forks *forks = forks_arg;
    pthread_mutex_lock(forks->left_fork);
    pthread_mutex_lock(forks->right_fork);

//  printf("Philosopher %d just ate.\n", forks->philosopher);
    pthread_mutex_unlock(forks->left_fork);
    pthread_mutex_unlock(forks->right_fork);
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread[NUM_THREADS];
    pthread_mutex_t mutex_resource[NUM_THREADS];

    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        // ANSI C/C++ require the cast to pthread_mutex_t, 'struct forks',
        //  respectively, because these are runtime statements, and not declarations
        //    mutex_resource[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&mutex_resource[i], NULL);
        forks[i] = (struct forks){i,
                                  &mutex_resource[i], &mutex_resource[(i+1) % NUM_THREADS]};
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&thread[i], NULL, &philosopher_doit, &forks[i]);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread[i], NULL);
    }

    return 0;
}
