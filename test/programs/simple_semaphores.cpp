#include <pthread.h>
#include "GMAL.h"
#include "GMALWrappers.h"

#define START_NUM 5

sem_t sem;

int main(int argc, char* argv[])
{
    gmal_init();


    gmal_sem_init(&sem, 0, START_NUM);

    for( int i = 0; i < START_NUM; i++) {
        gmal_sem_wait(&sem);
    }
    gmal_sem_post(&sem);
    gmal_sem_post(&sem);
    gmal_sem_wait(&sem);
    gmal_sem_wait(&sem);

    return 0;
}

