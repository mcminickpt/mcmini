#include <pthread.h>
#include <semaphore.h>

#define START_NUM 5

sem_t sem1, sem2;
pthread_t thread1, thread2;

void * thread1_doit(void *forks_arg) {
    sem_wait(&sem2);
    sem_wait(&sem2);
    sem_post(&sem1);
    return NULL;
}

void * thread2_doit(void *forks_arg) {
    for( int i = 0; i < START_NUM+1; i++) {
        sem_wait(&sem1);
    }
    sem_post(&sem2);
    return NULL;
}

int main(int argc, char* argv[]) {
    sem_init(&sem1, 0, START_NUM);
    sem_init(&sem2, 0, 1);

    pthread_create(&thread1, NULL, &thread1_doit, NULL);
    pthread_create(&thread2, NULL, &thread2_doit, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}

