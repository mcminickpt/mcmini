#include <pthread.h>
#include <unistd.h>
#include "GMAL.h"
#include "GMALWrappers.h"

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

pthread_t thread1, thread2;

void * thread_doit(void *forks_arg) {
    int len = (int) ((drand48() * 5) + 1);
    sleep(len);
    return nullptr;
}


int main(int argc, char* argv[])
{
    gmal_init();

    gmal_pthread_create(&thread1, NULL, &thread_doit, NULL);
    gmal_pthread_create(&thread2, NULL, &thread_doit, NULL);

    gmal_pthread_join(thread1, NULL);
    gmal_pthread_join(thread2, NULL);

    return 0;
}

