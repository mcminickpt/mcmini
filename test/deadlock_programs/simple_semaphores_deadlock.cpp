#include <pthread.h>
#include "MCMINI.h"
#include "MCMINIWrappers.h"

sem_t sem;

int main(int argc, char* argv[])
{
    gmal_init();


    gmal_sem_init(&sem, 0, 0);

    gmal_sem_post(&sem);
    gmal_sem_post(&sem);
    gmal_sem_wait(&sem);
    gmal_sem_wait(&sem);
    gmal_sem_wait(&sem);

    return 0;
}

