// Simple cond example

#include <unistd.h>
#include <pthread.h>
#include "MCMINI.h"
#include "MCMINIWrappers.h"

#define THREAD_NUM 5

pthread_mutex_t mutex, mutex_start;
pthread_cond_t cond;
pthread_t thread[THREAD_NUM];

void * thread_doit(void *unused)
{
    mc_pthread_mutex_lock(&mutex);
    mc_pthread_mutex_unlock(&mutex_start);
    mc_pthread_cond_wait(&cond, &mutex);
    mc_pthread_mutex_unlock(&mutex);
    return nullptr;
}

int main(int argc, char* argv[])
{
    mc_init();

    mc_pthread_mutex_init(&mutex, NULL);
    mc_pthread_mutex_init(&mutex_start, NULL);

    mc_pthread_cond_init(&cond, NULL);

    for(int i = 0; i < THREAD_NUM; i++) {
        mc_pthread_create(&thread[i], NULL, &thread_doit, NULL);
    }

    mc_pthread_mutex_lock(&mutex_start);
    for(int i = 0; i < THREAD_NUM; i++) {
        mc_pthread_mutex_lock(&mutex_start);
    }
    mc_pthread_mutex_unlock(&mutex_start);

    mc_pthread_mutex_lock(&mutex);
    mc_pthread_cond_broadcast(&cond);
    mc_pthread_mutex_unlock(&mutex);

    for(int i = 0; i < THREAD_NUM; i++) {
        mc_pthread_join(thread[i], NULL);
    }


    return 0;
}
