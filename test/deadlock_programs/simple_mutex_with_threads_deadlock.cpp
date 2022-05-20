#include <pthread.h>
#include "MCMINI.h"
#include "MCMINIWrappers.h"

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_t thread1, thread2;

void * thread_doit1(void *forks_arg) {
    gmal_pthread_mutex_lock(&mutex2);
    gmal_pthread_mutex_lock(&mutex1);
    gmal_pthread_mutex_unlock(&mutex1);
    gmal_pthread_mutex_unlock(&mutex2);
    return nullptr;
}

void * thread_doit2(void *forks_arg) {
    gmal_pthread_mutex_lock(&mutex1);
    gmal_pthread_mutex_lock(&mutex2);
    gmal_pthread_mutex_unlock(&mutex2);
    gmal_pthread_mutex_unlock(&mutex1);
    return nullptr;
}

int main(int argc, char* argv[])
{

    gmal_init();

    gmal_pthread_mutex_init(&mutex1, NULL);
    gmal_pthread_mutex_init(&mutex2, NULL);

    gmal_pthread_create(&thread1, NULL, &thread_doit1, NULL);
    gmal_pthread_create(&thread2, NULL, &thread_doit2, NULL);

    gmal_pthread_join(thread1, NULL);
    gmal_pthread_join(thread2, NULL);

    return 0;
}

