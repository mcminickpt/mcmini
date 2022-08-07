//#include <semaphore.h>
#include "mcmini/MCMINIWrappers.h"
#include "mcmini/MCMINI_Private.h"

/*
This program provides a possible solution for producer-consumer
problem using mutex and mc_semaphore. I have used 5 producers and 5
consumers to demonstrate the solution. You can always play with these
values.
*/

#define MaxItems \
  1 // Maximum items a producer can produce or a consumer can consume
#define BufferSize    1 // Size of the buffer
#define NUM_PRODUCERS 1
#define NUM_CONSUMERS 1

sem_t empty;
sem_t full;
int in  = 0;
int out = 0;
int buffer[BufferSize];
pthread_mutex_t mutex;

void *
producer(void *pno)
{
  int item;
  //    while (1) {
  item = rand(); // Produce an random item
  mc_sem_wait(&empty);
  mc_pthread_mutex_lock(&mutex);
  buffer[in] = item;
  in         = (in + 1) % BufferSize;
  mc_pthread_mutex_unlock(&mutex);
  mc_sem_post(&full);
  //    }

  return NULL;
}

void *
consumer(void *cno)
{
  for (int i = 0; i < MaxItems; i++) {
    mc_sem_wait(&full);
    mc_pthread_mutex_lock(&mutex);
    int item = buffer[out];
    out      = (out + 1) % BufferSize;
    mc_pthread_mutex_unlock(&mutex);
    mc_sem_post(&empty);
  }
  return NULL;
}

int
main()
{
  mc_init();
  pthread_t pro[5], con[5];
  mc_pthread_mutex_init(&mutex, NULL);
  mc_sem_init(&empty, 0, BufferSize);
  mc_sem_init(&full, 0, 0);

  int a[5] = {1, 2, 3, 4,
              5}; // Just used for numbering the producer and consumer

  for (int i = 0; i < NUM_PRODUCERS; i++) {
    mc_pthread_create(&pro[i], NULL, producer, (void *)&a[i]);
  }
  for (int i = 0; i < NUM_CONSUMERS; i++) {
    mc_pthread_create(&con[i], NULL, consumer, (void *)&a[i]);
  }

  for (int i = 0; i < NUM_PRODUCERS; i++) {
    mc_pthread_join(pro[i], NULL);
  }
  for (int i = 0; i < NUM_CONSUMERS; i++) {
    mc_pthread_join(con[i], NULL);
  }
  return 0;
}

//// Naive dining philosophers solution, which leads to deadlock.
//
//#include <stdio.h>
//#include <unistd.h>
//#include <pthread.h>
//#include "mcmini/MCMINI.h"
//#include "mcmini/MCMINIWrappers.h"
//
//#define NUM_THREADS 4
//
// struct forks {
//    int philosopher;
//    pthread_mutex_t *left_fork;
//    pthread_mutex_t *right_fork;
//} forks[NUM_THREADS];
//
// void * philosopher_doit(void *forks_arg) {
//    struct forks *forks = (struct forks*)forks_arg;
//    mc_pthread_mutex_lock(forks->left_fork);
//    mc_pthread_mutex_lock(forks->right_fork);
//    mc_pthread_mutex_unlock(forks->left_fork);
//    mc_pthread_mutex_unlock(forks->right_fork);
//    return NULL;
//}
//
// int main(int argc, char* argv[]) {
//    mc_init();
//
//    pthread_t thread[NUM_THREADS];
//    pthread_mutex_t mutex_resource[NUM_THREADS];
//
//    int i;
//    for (i = 0; i < NUM_THREADS; i++) {
//        // ANSI C/C++ require the cast to pthread_mutex_t, 'struct
//        forks',
//        //  respectively, because these are runtime statements, and
//        not declarations
//        //    mutex_resource[i] =
//        (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
//        mc_pthread_mutex_init(&mutex_resource[i], NULL);
//        forks[i] = (struct forks){i,
//                                  &mutex_resource[i],
//                                  &mutex_resource[(i+1) %
//                                  NUM_THREADS]};
//    }
//
//    for (i = 0; i < NUM_THREADS; i++) {
//        mc_pthread_create(&thread[i], NULL, &philosopher_doit,
//        &forks[i]);
//    }
//
//    for (i = 0; i < NUM_THREADS; i++) {
//        mc_pthread_join(thread[i], NULL);
//    }
//
//    return 0;
//}
