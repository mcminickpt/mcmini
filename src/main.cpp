#include "GMAL.h"
#include "GMALWrappers.h"
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

/*
This program provides a possible solution for producer-consumer problem using mutex and semaphore.
I have used 5 producers and 5 consumers to demonstrate the solution. You can always play with these values.
*/

#define NUM_PRODUCERS 1
#define NUM_CONSUMERS 1
#define MaxItems 1 // Maximum items a producer can produce or a consumer can consume
#define BufferSize 5 // Size of the buffer

sem_t empty;
sem_t full;
int in = 0;
int out = 0;
int buffer[BufferSize];
pthread_mutex_t mutex;

void *producer(void *pno)
{
//    gmal_exit(0);
    int i;
    int item;
    for( i = 0; i < MaxItems*2; i++) {
        item = rand(); // Produce an random item
        gmal_sem_wait(&empty);
        gmal_pthread_mutex_lock(&mutex);
        buffer[in] = item;
        printf("Producer %d: Insert Item %d at %d\n", *((int *)pno),buffer[in],in);
        in = (in+1)%BufferSize;
        gmal_pthread_mutex_unlock(&mutex);
        gmal_sem_post(&full);
    }
}

void *consumer(void *cno)
{
//    gmal_exit(0);
    int i;
    for( i = 0; i < MaxItems; i++) {
        gmal_sem_wait(&full);
        gmal_pthread_mutex_lock(&mutex);
        int item = buffer[out];
        printf("Consumer %d: Remove Item %d from %d\n",*((int *)cno),item, out);
        out = (out+1)%BufferSize;
        gmal_pthread_mutex_unlock(&mutex);
        gmal_sem_post(&empty);
    }
}

int main()
{
    gmal_init();
    int i;
    pthread_t pro[5], con[5];
    gmal_pthread_mutex_init(&mutex, NULL);
    gmal_sem_init(&empty,0,BufferSize);
    gmal_sem_init(&full,0,0);

    int a[5] = {1,2,3, 4, 5}; //Just used for numbering the producer and consumer

    for( i = 0; i < NUM_PRODUCERS; i++) {
        gmal_pthread_create(&pro[i], NULL, producer, (void *)&a[i]);
    }
    for( i = 0; i < NUM_CONSUMERS; i++) {
        gmal_pthread_create(&con[i], NULL, consumer, (void *)&a[i]);
    }

    for( i = 0; i < NUM_PRODUCERS; i++) {
        gmal_pthread_join(pro[i], NULL);
    }
    for( i = 0; i < NUM_CONSUMERS; i++) {
        gmal_pthread_join(con[i], NULL);
    }

//    pthread_mutex_destroy(&mutex);
//    sem_destroy(&empty);
//    sem_destroy(&full);

    return 0;
}

//#include "GMAL.h"
//#include "GMALWrappers.h"
//#include <semaphore.h>
//#include <stdlib.h>
//#include <stdio.h>
//
///*
//This program provides a possible solution for producer-consumer problem using mutex and semaphore.
//I have used 5 producers and 5 consumers to demonstrate the solution. You can always play with these values.
//*/
//
//#define MaxItems 1 // Maximum items a producer can produce or a consumer can consume
//#define BufferSize 5 // Size of the buffer
//
//sem_t sem;
//int in = 0;
//int out = 0;
//int buffer[BufferSize];
//pthread_mutex_t mutex;
//
//void *thread2(void *cno)
//{
//    gmal_sem_wait(&sem);
//    gmal_sem_post(&sem);
//}
//
//void *thread3(void *cno)
//{
//    gmal_sem_wait(&sem);
//    gmal_sem_wait(&sem);
//    gmal_sem_post(&sem);
//}
//
//int main()
//{
//    gmal_init();
//    pthread_t pt;
//    pthread_t pt2;
////    gmal_pthread_mutex_init(&mutex, NULL);
//    gmal_sem_init(&sem, 0, 1);
//
//    gmal_pthread_create(&pt, NULL, thread2, NULL);
//    gmal_pthread_create(&pt2, NULL, thread3, NULL);
//
//    gmal_sem_wait(&sem);
//
//    return 0;
//}