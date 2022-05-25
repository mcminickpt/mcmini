#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "../CustomConditionVariable.h"

#define THREAD_NUM 2

pthread_mutex_t mutex;
sem_t sem;
custom_cond cond;
pthread_t thread[THREAD_NUM];

void * thread_doit(void *unused)
{
    pthread_mutex_lock(&mutex);
    sem_post(&sem);
    custom_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_mutex_init(&mutex, NULL);
    sem_init(&sem, 0, 0);

    custom_cond_init(&cond);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_create(&thread[i], NULL, &thread_doit, NULL);
    }

    for( int i = 0; i < THREAD_NUM - 1; i++) {
        sem_wait(&sem);
    }

    pthread_mutex_lock(&mutex);
    custom_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_join(thread[i], NULL);
    }

    return 0;
}
