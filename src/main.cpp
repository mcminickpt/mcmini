#include <semaphore.h>
#include "MCMINIWrappers.h"
#include "MCMINI.h"

/*
This program provides a possible solution for producer-consumer problem using mutex and mc_semaphore.
I have used 5 producers and 5 consumers to demonstrate the solution. You can always play with these values.
*/

#define MaxItems 2 // Maximum items a producer can produce or a consumer can consume
#define BufferSize 2 // Size of the buffer
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 1

sem_t empty;
sem_t full;
int in = 0;
int out = 0;
int buffer[BufferSize];
pthread_mutex_t mutex;

void *producer(void *pno)
{
    int item;
    for(int i = 0; i < MaxItems; i++) {
        item = rand(); // Produce an random item
        mc_sem_wait(&empty);
        mc_pthread_mutex_lock(&mutex);
        buffer[in] = item;
        in = (in+1)%BufferSize;
        mc_pthread_mutex_unlock(&mutex);
        mc_sem_post(&full);
    }
    GOAL();
    return NULL;
}

void *consumer(void *cno)
{
    GOAL();
    for(int i = 0; i < MaxItems; i++) {
        mc_sem_wait(&full);
        mc_pthread_mutex_lock(&mutex);
        int item = buffer[out];
        out = (out+1)%BufferSize;
        mc_pthread_mutex_unlock(&mutex);
        mc_sem_post(&empty);
    }
    return NULL;
}

int main() {
    mc_init();
//    GOAL();
    pthread_t pro[5],con[5];
    mc_pthread_mutex_init(&mutex, NULL);
    mc_sem_init(&empty,0,BufferSize);
    mc_sem_init(&full,0,0);

    int a[5] = {1,2,3,4,5}; //Just used for numbering the producer and consumer

    for(int i = 0; i < NUM_PRODUCERS; i++) {
        mc_pthread_create(&pro[i], NULL, producer, (void *)&a[i]);
    }
    for(int i = 0; i < NUM_CONSUMERS; i++) {
        mc_pthread_create(&con[i], NULL, consumer, (void *)&a[i]);
    }

    for(int i = 0; i < NUM_PRODUCERS; i++) {
        mc_pthread_join(pro[i], NULL);
    }
    for(int i = 0; i < NUM_CONSUMERS; i++) {
        mc_pthread_join(con[i], NULL);
    }
    return 0;
}
