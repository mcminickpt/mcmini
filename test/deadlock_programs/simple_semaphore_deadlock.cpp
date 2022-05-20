#include <pthread.h>
#include "MCMINI.h"
#include "MCMINIWrappers.h"

sem_t sem1;
sem_t sem2;
pthread_t thread1, thread2;

void * thread_doit1(void *forks_arg) {
    gmal_sem_wait(&sem2);
    gmal_sem_wait(&sem1);
    gmal_sem_post(&sem1);
    gmal_sem_post(&sem2);
    return nullptr;
}

void * thread_doit2(void *forks_arg) {
    gmal_sem_wait(&sem1);
    gmal_sem_wait(&sem2);
    gmal_sem_post(&sem2);
    gmal_sem_post(&sem1);
    return nullptr;
}

int main(int argc, char* argv[])
{

    gmal_init();

    gmal_sem_init(&sem1, 0, 1);
    gmal_sem_init(&sem2, 0, 1);

    gmal_pthread_create(&thread1, NULL, &thread_doit1, NULL);
    gmal_pthread_create(&thread2, NULL, &thread_doit2, NULL);

    gmal_pthread_join(thread1, NULL);
    gmal_pthread_join(thread2, NULL);

    return 0;
}

