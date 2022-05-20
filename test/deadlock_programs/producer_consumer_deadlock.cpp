#include "MCMINI.h"
#include "MCMINIWrappers.h"
#include <stdlib.h>
#include <stdio.h>

/*
This program provides a possible solution for producer-consumer problem using mutex and semaphore.
I have used 5 producers and 5 consumers to demonstrate the solution. You can always play with these values.
*/

#define MaxItems 5 // Maximum items a producer can produce or a consumer can consume
#define BufferSize 5 // Size of the buffer

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
        gmal_pthread_mutex_lock(&mutex);
        gmal_sem_wait(&empty);
        buffer[in] = item;
        printf("Producer %d: Insert Item %d at %d\n", *((int *)pno),buffer[in],in);
        in = (in+1)%BufferSize;
        gmal_pthread_mutex_unlock(&mutex);
        gmal_sem_post(&full);
    }
    return nullptr;
}
void *consumer(void *cno)
{
    for(int i = 0; i < MaxItems; i++) {
        gmal_pthread_mutex_lock(&mutex);
        gmal_sem_wait(&full);
        int item = buffer[out];
        printf("Consumer %d: Remove Item %d from %d\n",*((int *)cno),item, out);
        out = (out+1)%BufferSize;
        gmal_pthread_mutex_unlock(&mutex);
        gmal_sem_post(&empty);
    }
    return nullptr;
}

int main()
{
    pthread_t pro[5],con[5];

    gmal_init();
    gmal_pthread_mutex_init(&mutex, NULL);
    gmal_sem_init(&empty,0,BufferSize);
    gmal_sem_init(&full,0,0);

    int a[5] = {1,2,3,4,5}; //Just used for numbering the producer and consumer

    for(int i = 0; i < 5; i++) {
        gmal_pthread_create(&pro[i], NULL, producer, (void *)&a[i]);
    }
    for(int i = 0; i < 5; i++) {
        gmal_pthread_create(&con[i], NULL, consumer, (void *)&a[i]);
    }

    for(int i = 0; i < 5; i++) {
        gmal_pthread_join(pro[i], NULL);
    }
    for(int i = 0; i < 5; i++) {
        gmal_pthread_join(con[i], NULL);
    }

    return 0;

}
