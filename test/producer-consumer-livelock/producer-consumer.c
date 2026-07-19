// This is a partial implementation of producer-consumer.  Fill in the rest.

// For debugging in GDB with threads: methodology 1
//   (gdb) info threads
//   (gdb) thread 2
//   (gdb) # and so on for other threads
//   (gdb) where # each thread has its own stack
//   (gdb) frame 2  # to go to call frame 2

// For debugging in GDB: methodology 2
//   (gdb) break consumer
//   (gdb) run
//   (gdb) print buf
//   (gdb) next
//   (gdb) print buf  # and so on

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h> // Needed for pthread_mutex_lock(), etc.
#include "MCProgress.h"
// This many producer and this many consumer threads.
#define NUM_PROD_THREADS 3
#define NUM_CONS_THREADS 3

// You have to:
// 1.  declare sem_t for your implementation
// 2.  Add to 'main':  sem_init, pthread_create, pthread_join
// 3.  define push_buf() and take_from_buf()
// 4.  link this with your semaphore.o (for your semaphore.c file),
//     which should defines sem_init, sem_wait, sem_post.
//     Note that 'partial_solution.c' is available for you to implement
//     sem_init, sem_wait, sem_post well enough for 1 producer, 1 consumer.

struct sem {
  int count;
  pthread_mutex_t mutex;
};
typedef struct sem sem_t;

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);

sem_t sem_producer;  // Should count number of empty slots available
sem_t sem_consumer;  // Should count number of items in the buffer
// pthread_mutex_t mut_buf = PTHREAD_MUTEX_INITIALIZER;  // Lock for anybody touching buf
pthread_mutex_t mut_buf;  // Lock for anybody touching buf

pthread_t prod_thread[NUM_PROD_THREADS];
pthread_t cons_thread[NUM_CONS_THREADS];
void *producer(void *arg);
void *consumer(void *arg);

// =========================================
// Implement circular buffer 'buf'
#define BUF_EMPTY_ITEM (-1)
#define BUF_NUM_ITEMS 2
// #define TRACE
int buf[BUF_NUM_ITEMS];
int push_idx = 0;
int take_idx = 0;
void initialize_buf() {
  int i;
  for (i = 0; i < BUF_NUM_ITEMS; i++) {
    buf[i] = BUF_EMPTY_ITEM;
  }
}
void push_buf(int work_item) {
  assert(buf[push_idx] == BUF_EMPTY_ITEM);
  buf[push_idx] = work_item;
#ifdef TRACE
  fprintf(stderr, "PUSH: buf[%d] = %d\n", push_idx, work_item);
#endif
  push_idx = (push_idx+1) % BUF_NUM_ITEMS;
}
int take_from_buf() {
  int work_item = buf[take_idx];
  buf[take_idx] = BUF_EMPTY_ITEM;
#ifdef TRACE
  fprintf(stderr, "TAKE: work_item:%d = buf[%d]\n", work_item, take_idx);
#endif
  assert(work_item != BUF_EMPTY_ITEM);
  take_idx = (take_idx+1) % BUF_NUM_ITEMS;
  return work_item;
}

// =========================================
// Implement producer-consumer, using buffer 'buf'.

int main() {
  // ... uses pthread_create to start producer and consumer
  // You must add that.
  initialize_buf(); // Initialize entries in buffer to '-1' for error checking.
  pthread_mutex_init(&(mut_buf), NULL); 
  sem_init(&sem_producer, 0, BUF_NUM_ITEMS);
  sem_init(&sem_consumer, 0, 0);
  int i;
  for (i = 0; i < NUM_PROD_THREADS; i++) {
    pthread_create(&prod_thread[i], NULL, producer, NULL);
  }
  for (i = 0; i < NUM_CONS_THREADS; i++) {
    pthread_create(&cons_thread[i], NULL, consumer, NULL);
  }
  for (i = 0; i < NUM_PROD_THREADS; i++) {
    pthread_join(prod_thread[i], NULL);
  }
  for (i = 0; i < NUM_CONS_THREADS; i++) {
    pthread_join(cons_thread[i], NULL);
  }
  // WARNING:  the primary thread runs main().  When main exits, the primary
  //             thread exits, unless you call 'pthread_join()' to
  //             have 'main' wait on the other threads before exiting.
  while (1);  // Don't let the primary thread exit
}

void *producer(void *arg) {
  int work_item = 1;
  while (1) {
    // sleep( rand() % 5 );
    sem_wait(&sem_producer);  // Wait for empty slots
    pthread_mutex_lock(&mut_buf);
      push_buf(work_item++);  // inside critical section with mut_buf lock
      MC_PROGRESS();
    pthread_mutex_unlock(&mut_buf);
    sem_post(&sem_consumer);  // Tell the consumer there's a new work item
  }
}

// Exactly the same, but the inverse:
void *consumer(void *arg) {
  while (1) {
    int work_item;
    // sleep( rand() % 5 );
    sem_wait(&sem_consumer);
    pthread_mutex_lock(&mut_buf);
      work_item = take_from_buf();
      MC_PROGRESS();
    pthread_mutex_unlock(&mut_buf);
    sem_post(&sem_producer);

    printf("%d ", work_item);
    fflush(stdout);  // Force printing now; don't wait for the newline
  }
}
