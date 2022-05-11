// Simple cond example

#include <unistd.h>
#include <pthread.h>
#include "GMAL.h"
#include "GMALWrappers.h"

#define THREAD_NUM 5

pthread_mutex_t mutex, mutex_start;
pthread_cond_t cond;
pthread_t thread[THREAD_NUM];

void * thread_doit(void *unused)
{
    gmal_pthread_mutex_lock(&mutex);
    gmal_pthread_mutex_unlock(&mutex_start);
    gmal_pthread_cond_wait(&cond, &mutex);
    gmal_pthread_mutex_unlock(&mutex);
    return nullptr;
}

int main(int argc, char* argv[])
{
    gmal_init();

    gmal_pthread_mutex_init(&mutex, NULL);
    gmal_pthread_mutex_init(&mutex_start, NULL);

    gmal_pthread_cond_init(&cond, NULL);

    for(int i = 0; i < THREAD_NUM; i++) {
        gmal_pthread_create(&thread[i], NULL, &thread_doit, NULL);
    }

    gmal_pthread_mutex_lock(&mutex_start);
    for(int i = 0; i < THREAD_NUM; i++) {
        gmal_pthread_mutex_lock(&mutex_start);
    }
    gmal_pthread_mutex_unlock(&mutex_start);

    gmal_pthread_mutex_lock(&mutex);
    gmal_pthread_cond_broadcast(&cond);
    gmal_pthread_mutex_unlock(&mutex);

    for(int i = 0; i < THREAD_NUM; i++) {
        gmal_pthread_join(thread[i], NULL);
    }


    return 0;
}
