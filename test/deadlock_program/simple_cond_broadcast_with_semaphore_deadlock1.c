#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

int THREAD_NUM;
int DEBUG = 0;

pthread_mutex_t mutex;
sem_t sem;
pthread_cond_t cond;
pthread_t *thread;

void * thread_doit(void *unused)
{
    pthread_mutex_lock(&mutex);
    sem_post(&sem);
    if(DEBUG) printf("Thread %ld: Sent signal to semaphore\n", pthread_self());
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[]) {
    if(argc != 3){
        printf("Usage: %s THREAD_NUM DEBUG\n", argv[0]);
        return 1;
    }

    THREAD_NUM = atoi(argv[1]);
    DEBUG = atoi(argv[2]);

    thread = (pthread_t*) malloc(THREAD_NUM * sizeof(pthread_t));

    pthread_mutex_init(&mutex, NULL);
    sem_init(&sem, 0, 0);

    pthread_cond_init(&cond, NULL);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_create(&thread[i], NULL, &thread_doit, NULL);
    }

    for( int i = 0; i < THREAD_NUM + 1; i++) {
        sem_wait(&sem);
    }

    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_join(thread[i], NULL);
    }

    free(thread);
    return 0;
}
