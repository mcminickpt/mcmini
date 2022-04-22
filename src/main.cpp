#include "GMAL.h"
#include "GMALWrappers.h"
#include "transitions/GMALThreadStart.h"
#include <pthread.h>

//pthread_t main_thread;

//pthread_mutex_t m1;
//pthread_mutex_t m2;
//
//
//void*
//thread_helper_main(void *unused) {
//    gmal_pthread_mutex_lock(&m2);
//    gmal_pthread_mutex_lock(&m1);
//    return NULL;
//}
//
//int
//main(int argc, const char **argv)
//{
//    gmal_init();
//    pthread_t helper;
//
//    gmal_pthread_mutex_init(&m1, nullptr);
//    gmal_pthread_mutex_init(&m2, nullptr);
//    gmal_pthread_create(&helper, nullptr, &thread_helper_main, nullptr);
////    gmal_pthread_create(&helper, nullptr, &thread_helper_main, nullptr);
//    gmal_pthread_mutex_lock(&m1);
//    gmal_pthread_mutex_lock(&m2);
//    gmal_exit_main_thread();
//    return 0;
//}

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 3

struct forks {
    int philosopher;
    pthread_mutex_t *left_fork;
    pthread_mutex_t *right_fork;
} forks[NUM_THREADS];

void * philosopher_doit(void *forks_arg) {
    struct forks *forks = static_cast<struct forks *>(forks_arg);
//    printf("forks %p, %p, %p\n", forks, forks->left_fork, forks->right_fork);
    gmal_pthread_mutex_lock(forks->left_fork);
    gmal_pthread_mutex_lock(forks->right_fork);
    gmal_pthread_mutex_unlock(forks->left_fork);
    gmal_pthread_mutex_unlock(forks->right_fork);
    return NULL;
}

int main(int argc, char* argv[])
{
    gmal_init();
    pthread_t thread[NUM_THREADS];
    pthread_mutex_t mutex_resource[NUM_THREADS];

    int i;
    for (i = 0; i < NUM_THREADS; i++) {
        // ANSI C/C++ require the cast to pthread_mutex_t, 'struct forks',
        //  respectively, because these are runtime statements, and not declarations
//    mutex_resource[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        gmal_pthread_mutex_init(&mutex_resource[i], nullptr);
        forks[i] = (struct forks){i,
                                  &mutex_resource[i], &mutex_resource[(i+1) % NUM_THREADS]};
    }

    for (i = 0; i < NUM_THREADS; i++) {
        gmal_pthread_create(&thread[i], nullptr, &philosopher_doit, &forks[i]);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        gmal_pthread_join(thread[i], NULL);
    }

//    gmal_exit_main_thread();
    return 0;
}