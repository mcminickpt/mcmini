#include <pthread.h>
#include "MCMINI.h"
#include "MCMINIWrappers.h"

#define START_NUM 5

sem_t sem1, sem2;
pthread_t thread1, thread2;

void * thread1_doit(void *forks_arg) {
    mc_sem_wait(&sem2);
    mc_sem_wait(&sem2);
    mc_sem_post(&sem1);
    return nullptr;
}

void * thread2_doit(void *forks_arg) {
    for( int i = 0; i < START_NUM+1; i++) {
        mc_sem_wait(&sem1);
    }
    mc_sem_post(&sem2);
    return nullptr;
}

int main(int argc, char* argv[])
{
    mc_init();

    mc_sem_init(&sem1, 0, START_NUM);
    mc_sem_init(&sem2, 0, 1);

    mc_pthread_create(&thread1, NULL, &thread1_doit, NULL);
    mc_pthread_create(&thread2, NULL, &thread2_doit, NULL);

    mc_pthread_join(thread1, NULL);
    mc_pthread_join(thread2, NULL);

    return 0;
}

