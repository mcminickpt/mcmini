//#include <stdlib.h>
//#include <stdio.h>
//#include <mc_pthread.h>
//#include <mc_semaphore.h>
//#include "MCMINI.h"
//#include "MCMINIWrappers.h"
//
///*
//This program provides a possible solution for producer-consumer problem using mutex and mc_semaphore.
//I have used 5 producers and 5 consumers to demonstrate the solution. You can always play with these values.
//*/
//
//#define MaxItems 5 // Maximum items a producer can produce or a consumer can consume
//#define BufferSize 5 // Size of the buffer
//
//mc_sem_t empty;
//mc_sem_t full;
//int in = 0;
//int out = 0;
//int buffer[BufferSize];
//mc_pthread_mutex_t mutex;
//
//void *producer(void *pno)
//{
//    int item;
//    for(int i = 0; i < MaxItems; i++) {
//        item = rand(); // Produce an random item
//        mc_mc_pthread_mutex_lock(&mutex);
//        mc_mc_sem_wait(&empty);
//        buffer[in] = item;
//        in = (in+1)%BufferSize;
//        mc_mc_pthread_mutex_unlock(&mutex);
//        mc_mc_sem_post(&full);
//        GOAL();
//    }
//    return NULL;
//}
//void *consumer(void *cno)
//{
//    for(int i = 0; i < MaxItems; i++) {
//        mc_mc_pthread_mutex_lock(&mutex);
//        mc_mc_sem_wait(&full);
//        int item = buffer[out];
//        out = (out+1)%BufferSize;
//        mc_mc_pthread_mutex_unlock(&mutex);
//        mc_mc_sem_post(&empty);
//    }
//    return NULL;
//}
//
//int main() {
//    mc_pthread_t pro[5],con[5];
//    mc_init();
//
//    mc_mc_pthread_mutex_init(&mutex, NULL);
//    mc_mc_sem_init(&empty,0,BufferSize);
//    mc_mc_sem_init(&full,0,0);
//
//    int a[5] = {1,2,3,4,5}; //Just used for numbering the producer and consumer
//
//    for(int i = 0; i < 1; i++) {
//        mc_mc_pthread_create(&pro[i], NULL, producer, (void *)&a[i]);
//    }
//    for(int i = 0; i < 1; i++) {
//        mc_mc_pthread_create(&con[i], NULL, consumer, (void *)&a[i]);
//    }
//
//    for(int i = 0; i < 1; i++) {
//        mc_mc_pthread_join(pro[i], NULL);
//    }
//    for(int i = 0; i < 1; i++) {
//        mc_mc_pthread_join(con[i], NULL);
//    }
//
//    return 0;
//}

//#include <unistd.h>
//#include <pthread.h>
//#include <semaphore.h>
//#include "MCMINI.h"
//#include "MCMINIWrappers.h"
//
//#define THREAD_NUM 2
//
//pthread_mutex_t mutex;
//sem_t mc_sem;
//pthread_cond_t cond;
//pthread_t thread[THREAD_NUM];
//
//void * thread_doit(void *unused)
//{
//    mc_pthread_mutex_lock(&mutex);
//    mc_sem_post(&mc_sem);
//    mc_pthread_cond_wait(&cond, &mutex);
//    mc_pthread_mutex_unlock(&mutex);
//    return NULL;
//}
//
//int main(int argc, char* argv[]) {
//    mc_init();
//    mc_pthread_mutex_init(&mutex, NULL);
//    mc_sem_init(&mc_sem, 0, 0);
//
//    mc_pthread_cond_init(&cond, NULL);
//
//    for(int i = 0; i < THREAD_NUM; i++) {
//        mc_pthread_create(&thread[i], NULL, &thread_doit, NULL);
//    }
//
//    for( int i = 0; i < THREAD_NUM + 1; i++) {
//        mc_sem_wait(&mc_sem);
//    }
//
//    mc_pthread_mutex_lock(&mutex);
//    mc_pthread_cond_broadcast(&cond);
//    mc_pthread_mutex_unlock(&mutex);
//
//    for(int i = 0; i < THREAD_NUM; i++) {
//        mc_pthread_join(thread[i], NULL);
//    }
//
//    return 0;
//}


//#include <unistd.h>
//#include <pthread.h>
//#include <semaphore.h>
//#include "../test/CustomConditionVariable.h"
//
//#define THREAD_NUM 5
//
//pthread_mutex_t mutex;
//sem_t sem;
//custom_cond cond;
//pthread_t thread[THREAD_NUM];
//
//void * thread_doit(void *unused)
//{
//    pthread_mutex_lock(&mutex);
//    sem_post(&sem);
//    custom_cond_wait(&cond, &mutex);
//    pthread_mutex_unlock(&mutex);
//    return NULL;
//}
//
//int main(int argc, char* argv[]) {
//    pthread_mutex_init(&mutex, NULL);
//    sem_init(&sem, 0, 0);
//
//    custom_cond_init(&cond);
//
//    for(int i = 0; i < THREAD_NUM; i++) {
//        pthread_create(&thread[i], NULL, &thread_doit, NULL);
//    }
//
//    for( int i = 0; i < THREAD_NUM - 1; i++) {
//        sem_wait(&sem);
//    }
//
//    pthread_mutex_lock(&mutex);
//    custom_cond_broadcast(&cond);
//    pthread_mutex_unlock(&mutex);
//
//    for(int i = 0; i < THREAD_NUM; i++) {
//        pthread_join(thread[i], NULL);
//    }
//
//    return 0;
//}

//#include <unistd.h>
//#include <pthread.h>
//#include <semaphore.h>
//
//extern "C" {
//#include "../test/CustomConditionVariable.h"
//}
//
//#define THREAD_NUM 2
//
//pthread_mutex_t mutex;
//sem_t sem;
//custom_cond cond;
//pthread_t thread[THREAD_NUM];
//
//void * thread_doit(void *unused)
//{
//    pthread_mutex_lock(&mutex);
//    sem_post(&sem);
//    custom_cond_wait(&cond, &mutex);
//    pthread_mutex_unlock(&mutex);
//    return NULL;
//}
//
//int main(int argc, char* argv[]) {
//    pthread_mutex_init(&mutex, NULL);
//    sem_init(&sem, 0, 0);
//
//    custom_cond_init(&cond);
//
//    for(int i = 0; i < THREAD_NUM; i++) {
////        pthread_create(&thread[i], NULL, &thread_doit, NULL);
//    }
//
//    for( int i = 0; i < THREAD_NUM - 1; i++) {
//        sem_wait(&sem);
//    }
//
////    pthread_mutex_lock(&mutex);
////    custom_cond_broadcast(&cond);
////    pthread_mutex_unlock(&mutex);
////
////    for(int i = 0; i < THREAD_NUM; i++) {
////        pthread_join(thread[i], NULL);
////    }
//
//    return 0;
//}

//#include <unistd.h>
//#include <pthread.h>
//#include <semaphore.h>
//
//#define THREAD_NUM 2
//
//sem_t sem1, sem2, sem3;
//pthread_t thread1, thread2;
//
//void *
//routine1(void *unused)
//{
//    sem_wait(&sem1);
//    sem_post(&sem2);
//    sem_post(&sem1);
//}
//
//void *
//routine2(void *unused)
//{
//    sem_wait(&sem1);
//    sem_wait(&sem2);
//    sem_post(&sem3);
//}
//
//int main(int argc, char* argv[]) {
//
//    sem_init(&sem1, 0, 0);
//    sem_init(&sem2, 0, 0);
//    sem_init(&sem3, 0, 0);
//
//    pthread_create(&thread1, NULL, &routine1, NULL);
//    pthread_create(&thread2, NULL, &routine2, NULL);
//
//    sem_wait(&sem3);
//
//    return 0;
//}

//#include <unistd.h>
//#include <pthread.h>
//#include <semaphore.h>
//#include "MCMINIWrappers.h"
//#include "MCMINI.h"
//
//#define THREAD_NUM 2
//
//sem_t sem1, sem2, sem3;
//pthread_t thread1, thread2;
//
//void *
//routine1(void *unused)
//{
//    mc_sem_wait(&sem1);
//    mc_sem_post(&sem2);
//    mc_sem_post(&sem1);
//}
//
//void *
//routine2(void *unused)
//{
//    mc_sem_wait(&sem1);
//    mc_sem_wait(&sem2);
//    mc_sem_post(&sem3);
//}
//
//int main(int argc, char* argv[]) {
//    mc_init();
//    mc_sem_init(&sem1, 0, 0);
//    mc_sem_init(&sem2, 0, 0);
//    mc_sem_init(&sem3, 0, 0);
//
//    mc_pthread_create(&thread1, NULL, &routine1, NULL);
//    mc_pthread_create(&thread2, NULL, &routine2, NULL);
//
//    mc_sem_post(&sem1);
//    mc_sem_wait(&sem3);
//
//    return 0;
//}

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "MCMINIWrappers.h"
#include "MCMINI.h"

#define THREAD_NUM 2

sem_t sem1, sem2, sem3;
pthread_t thread1, thread2;

void *
routine1(void *unused)
{
    mc_sem_wait(&sem1);
    mc_sem_wait(&sem1);
    mc_sem_wait(&sem2);
    GOAL();
    return NULL;
}

void *
routine2(void *unused)
{
    mc_sem_wait(&sem1);
    mc_sem_post(&sem2);
    mc_sem_post(&sem3);
    GOAL();
    return NULL;
}

int main(int argc, char* argv[]) {
    mc_init();
    mc_sem_init(&sem1, 0, 0);
    mc_sem_init(&sem2, 0, 0);
    mc_sem_init(&sem3, 0, 0);

    mc_pthread_create(&thread1, NULL, &routine1, NULL);
    mc_pthread_create(&thread2, NULL, &routine2, NULL);

    mc_sem_post(&sem1);
    mc_sem_post(&sem1);
    mc_sem_wait(&sem3);
    GOAL();

    return 0;
}