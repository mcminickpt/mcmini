#include <pthread.h>
#include "GMAL.h"
#include "GMALWrappers.h"

pthread_barrier_t barrier;

int main(int argc, char* argv[])
{
    gmal_init();
    gmal_pthread_barrier_init(barrier, NULL, 2);
    gmal_pthread_barrier_wait(barrier);
    return 0;
}

