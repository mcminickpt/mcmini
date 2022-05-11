#include <pthread.h>
#include "GMAL.h"
#include "GMALWrappers.h"

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

int main(int argc, char* argv[])
{
    gmal_init();

    gmal_pthread_mutex_init(&mutex1, NULL);
    gmal_pthread_mutex_init(&mutex2, NULL);

    gmal_pthread_mutex_lock(&mutex1);
    gmal_pthread_mutex_lock(&mutex2);
    gmal_pthread_mutex_unlock(&mutex2);
    gmal_pthread_mutex_unlock(&mutex1);

    return 0;
}

