// Simple cond example

#include <unistd.h>
#include <pthread.h>
#include "GMAL.h"
#include "GMALWrappers.h"


pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_t thread;


void * thread_doit(void *unused)
{
    gmal_pthread_mutex_lock(&mutex);
    gmal_pthread_cond_wait(&cond, &mutex);
    gmal_pthread_mutex_unlock(&mutex);
    return nullptr;
}

int main(int argc, char* argv[])
{
    gmal_init();

    gmal_pthread_mutex_init(&mutex, NULL);
    gmal_pthread_cond_init(&cond, NULL);

    gmal_pthread_create(&thread, NULL, &thread_doit, NULL);

    gmal_pthread_mutex_lock(&mutex);
    gmal_pthread_cond_signal(&cond);
    gmal_pthread_mutex_unlock(&mutex);

    gmal_pthread_join(thread, NULL);

    return 0;
}
