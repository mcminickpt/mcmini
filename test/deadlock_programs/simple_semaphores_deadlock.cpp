#include <pthread.h>
#include "MCMINI.h"
#include "MCMINIWrappers.h"

sem_t sem;

int main(int argc, char* argv[])
{
    mc_init();


    mc_sem_init(&sem, 0, 0);

    mc_sem_post(&sem);
    mc_sem_post(&sem);
    mc_sem_wait(&sem);
    mc_sem_wait(&sem);
    mc_sem_wait(&sem);

    return 0;
}

