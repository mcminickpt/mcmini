#include <pthread.h>
#include "../CustomBarrier.h"

#define THREAD_NUM 3

custom_barrier barrier;
pthread_t thread[THREAD_NUM];

void * thread_doit(void *unused)
{
    custom_barrier_wait(&barrier);
    return NULL;
}
int main(int argc, char* argv[]) {
    custom_barrier_init(&barrier, THREAD_NUM + 1);
    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_create(&thread[i], NULL, &thread_doit, NULL);
    }

   custom_barrier_wait(&barrier);

    for(int i = 0; i < THREAD_NUM; i++) {
        pthread_join(thread[i], NULL);
    }
    return 0;
}