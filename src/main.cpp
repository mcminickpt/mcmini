//// Naive dining philosophers solution, which leads to deadlock.
//
//#include <stdio.h>
//#include <unistd.h>
//#include <pthread.h>
//#include "GMAL.h"
//#include "GMALWrappers.h"
//
//#define NUM_THREADS 5
//
//struct forks {
//    int philosopher;
//    pthread_mutex_t *left_fork;
//    pthread_mutex_t *right_fork;
//} forks[NUM_THREADS];
//
//pthread_mutex_t dining_fork;
//
//void * philosopher_doit(void *forks_arg) {
//    struct forks *forks = static_cast<struct forks*>(forks_arg);
//    gmal_pthread_mutex_lock(forks->left_fork);
//    gmal_pthread_mutex_lock(forks->right_fork);
//
////  printf("Philosopher %d just ate.\n", forks->philosopher);
//    gmal_pthread_mutex_unlock(forks->left_fork);
//    gmal_pthread_mutex_unlock(forks->right_fork);
//    return NULL;
//}
//
//int main(int argc, char* argv[])
//{
//    gmal_init();
//    pthread_t thread[NUM_THREADS];
//    pthread_mutex_t mutex_resource[NUM_THREADS];
//
//    gmal_pthread_mutex_init(&dining_fork, NULL);
//
//    int i;
//    for (i = 0; i < NUM_THREADS; i++) {
//        // ANSI C/C++ require the cast to pthread_mutex_t, 'struct forks',
//        //  respectively, because these are runtime statements, and not declarations
////    mutex_resource[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
//        gmal_pthread_mutex_init(&mutex_resource[i], NULL);
//        forks[i] = (struct forks){i,
//                                  &mutex_resource[i], &mutex_resource[(i+1) % NUM_THREADS]};
//    }
//
//    for (i = 0; i < NUM_THREADS; i++) {
//        gmal_pthread_create(&thread[i], NULL, &philosopher_doit, &forks[i]);
//    }
//
//    for (i = 0; i < NUM_THREADS; i++) {
//        gmal_pthread_join(thread[i], NULL);
//    }
//
//    return 0;
//}

// Naive dining philosophers solution, which leads to deadlock.

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "GMAL.h"
#include "GMALWrappers.h"


pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_cond_t cond;
pthread_t thread;

void * philosopher_doit(void *forks_arg) {
    gmal_pthread_mutex_lock(&mutex2);
    gmal_pthread_mutex_lock(&mutex1);
    gmal_pthread_mutex_unlock(&mutex1);
    gmal_pthread_mutex_unlock(&mutex2);
    return nullptr;
}

void * test(void *unused)
{
    gmal_pthread_mutex_lock(&mutex1);
    gmal_pthread_cond_wait(&cond, &mutex1);
    gmal_pthread_mutex_unlock(&mutex1);
    return nullptr;
}

int main(int argc, char* argv[])
{
    gmal_init();

    gmal_pthread_mutex_init(&mutex1, NULL);
    gmal_pthread_cond_init(&cond, NULL);

    gmal_pthread_create(&thread, NULL, &test, NULL);

    gmal_pthread_mutex_lock(&mutex1);
    gmal_pthread_cond_signal(&cond);
    gmal_pthread_mutex_unlock(&mutex1);

    gmal_pthread_mutex_lock(&mutex1);
    gmal_pthread_mutex_unlock(&mutex1);

//    gmal_pthread_mutex_init(&mutex1, nullptr);
//    gmal_pthread_mutex_init(&mutex2, nullptr);
//
//    gmal_pthread_create(&thread, NULL, &philosopher_doit, NULL);
//
//    gmal_pthread_mutex_lock(&mutex1);
//    gmal_pthread_mutex_lock(&mutex2);
//    gmal_pthread_mutex_unlock(&mutex2);
//    gmal_pthread_mutex_unlock(&mutex1);

    return 0;
}
