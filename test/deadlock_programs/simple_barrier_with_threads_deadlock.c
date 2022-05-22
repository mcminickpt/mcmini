#include <pthread.h>

#define THREAD_NUM 5

pthread_barrier_t barrier;
pthread_t thread[THREAD_NUM];

void * thread_doit(void *unused)
{
    pthread_barrier_wait(&barrier);
    return NULL;
}
int main(int argc, char* argv[]) {
    pthread_barrier_init(&barrier, NULL, THREAD_NUM);
    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_create(&thread[i], NULL, &thread_doit, NULL);
    }

    pthread_barrier_wait(&barrier);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_join(thread[i], NULL);
    }
    return 0;
}