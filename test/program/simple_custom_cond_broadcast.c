#include <unistd.h>
#include <pthread.h>
#include "../CustomConditionVariable.h"

#define THREAD_NUM 3

pthread_mutex_t mutex, mutex_start;
custom_cond cond;
pthread_t thread[THREAD_NUM];

void * thread_doit(void *unused)
{
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex_start);
    custom_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[]) {

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_start, NULL);

    custom_cond_init(&cond);

    pthread_mutex_lock(&mutex);
    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_create(&thread[i], NULL, &thread_doit, NULL);
    }
    pthread_mutex_unlock(&mutex);

    pthread_mutex_lock(&mutex_start);
    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_mutex_lock(&mutex_start);
    }
    pthread_mutex_unlock(&mutex_start);

    pthread_mutex_lock(&mutex);
    custom_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_join(thread[i], NULL);
    }

    return 0;
}
