#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

pthread_t thread1, thread2;

void * thread_doit(void *forks_arg) {
    int len = (int) ((drand48() * 5) + 1);
    sleep(len);
    return NULL;
}

int main(int argc, char* argv[])
{
    pthread_create(&thread1, NULL, &thread_doit, NULL);
    pthread_create(&thread2, NULL, &thread_doit, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}

