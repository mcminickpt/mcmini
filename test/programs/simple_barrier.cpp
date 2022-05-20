#include <pthread.h>
#include "MCMINI.h"
#include "MCMINIWrappers.h"

pthread_barrier_t barrier;

int main(int argc, char* argv[])
{
    gmal_init();
    gmal_pthread_barrier_init(&barrier, NULL, 1);
    gmal_pthread_barrier_wait(&barrier);
    return 0;
}

