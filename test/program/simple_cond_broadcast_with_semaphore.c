#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

pthread_mutex_t mutex;
sem_t sem;
pthread_cond_t cond;

void * thread_doit(void *unused)
{
    pthread_mutex_lock(&mutex);
    sem_post(&sem);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[])
{
    if(argc < 2) {
        printf("Expected usage: %s THREAD_NUM\n", argv[0]);
        return -1;
    }

    int THREAD_NUM = atoi(argv[1]);

    pthread_t *threads = malloc(sizeof(pthread_t) * THREAD_NUM);

    pthread_mutex_init(&mutex, NULL);
    sem_init(&sem, 0, 0);
    pthread_cond_init(&cond, NULL);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_create(&threads[i], NULL, &thread_doit, NULL);
    }

    for(int i = 0; i < THREAD_NUM; i++) {
        sem_wait(&sem);
    }

    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&sem);
    pthread_cond_destroy(&cond);
    
    return 0;
}
