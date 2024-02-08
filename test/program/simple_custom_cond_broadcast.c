#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "../CustomConditionVariable.h"

pthread_mutex_t mutex, mutex_start;
custom_cond cond;

void * thread_doit(void *unused)
{
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex_start);
    custom_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[]) {

    if(argc < 2) {
        printf("Expected usage: %s THREAD_NUM\n", argv[0]);
        return -1;
    }

    int THREAD_NUM = atoi(argv[1]);

    pthread_t *threads = malloc(sizeof(pthread_t) * THREAD_NUM);

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_start, NULL);
    custom_cond_init(&cond);

    pthread_mutex_lock(&mutex);
    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_create(&threads[i], NULL, &thread_doit, NULL);
    }
    pthread_mutex_unlock(&mutex);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_mutex_lock(&mutex_start);
        pthread_mutex_unlock(&mutex_start);
    }

    pthread_mutex_lock(&mutex);
    custom_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_start);
    custom_cond_destroy(&cond);

    return 0;
}
