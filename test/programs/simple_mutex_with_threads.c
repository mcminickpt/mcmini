#include <pthread.h>
#include "MCMINI.h"
#include "MCMINIWrappers.h"

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_t thread1, thread2;

void * thread_doit(void *forks_arg) {
    mc_pthread_mutex_lock(&mutex1);
    mc_pthread_mutex_lock(&mutex2);
    mc_pthread_mutex_unlock(&mutex2);
    mc_pthread_mutex_unlock(&mutex1);
    return nullptr;
}


int main(int argc, char* argv[])
{
    mc_init();

    mc_pthread_mutex_init(&mutex1, NULL);
    mc_pthread_mutex_init(&mutex2, NULL);

    mc_pthread_create(&thread1, NULL, &thread_doit, NULL);
    mc_pthread_create(&thread2, NULL, &thread_doit, NULL);

    mc_pthread_join(thread1, NULL);
    mc_pthread_join(thread2, NULL);

    return 0;
}

