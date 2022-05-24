#include <pthread.h>
#include "MCMINI.h"
#include "MCMINIWrappers.h"

pthread_barrier_t barrier;
int shared_value = 0;

void *run(void* arg)
{
    int a = 10;
    WRITE(&shared_value, &a);
    return NULL;
}

int main(int argc, char* argv[])
{
    mc_init();
    pthread_t a;

    mc_pthread_create(&a, NULL, &run, NULL);
    READ(&shared_value);
    return 0;
}

