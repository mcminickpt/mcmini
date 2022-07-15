// Simple cond example

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define THREAD_NUM 5

pthread_mutex_t mutex;
sem_t sem;
pthread_cond_t cond;
pthread_t thread[THREAD_NUM];

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
    pthread_mutex_init(&mutex, NULL);
    sem_init(&sem, 0, 0);

    pthread_cond_init(&cond, NULL);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_create(&thread[i], NULL, &thread_doit, NULL);
    }

    for( int i = 0; i < THREAD_NUM; i++) {
        sem_wait(&sem);
    }

    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_join(thread[i], NULL);
    }

    return 0;
}
