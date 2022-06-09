#include <pthread.h>
#include <semaphore.h>

#define START_NUM 5

sem_t sem;

int main(int argc, char* argv[])
{
    sem_init(&sem, 0, START_NUM);

    for( int i = 0; i < START_NUM; i++) {
        sem_wait(&sem);
    }
    sem_post(&sem);
    sem_post(&sem);
    sem_wait(&sem);
    sem_wait(&sem);

    return 0;
}

