#include <unistd.h>
#include <pthread.h>

#define THREAD_NUM 2

pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_t thread[THREAD_NUM];

void * thread_doit(void *unused)
{
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_create(&thread[i], NULL, &thread_doit, NULL);
    }
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_join(thread[i], NULL);
    }

    return 0;
}
