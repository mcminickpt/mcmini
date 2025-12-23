#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 

#define NUM_WRITERS 3
#define ITERATIONS 1

int shared_data = 0; 
int writer_count_db1 = 0;
int writer_count_db2 = 0;
int waiting_writer_count_db1 = 0;
int waiting_writer_count_db2 = 0;
pthread_mutex_t mutex_lock_db1; 
pthread_mutex_t mutex_lock_db2; 
// One condition variable, but two mutexes
pthread_cond_t writers_cond; 

void *writer(void *not_used) {
  for (int i = 0; i < ITERATIONS; i++) {
    // We're doing two acquire-release paradigms,
    //   but with one condition variable and two mutexes.
    //   This is a bug.

    // ACQUIRE for db1
    pthread_mutex_lock(&mutex_lock_db1);
    waiting_writer_count_db1++;
    while (writer_count_db1 > 0) {
      pthread_cond_wait(&writers_cond, &mutex_lock_db1);
    }
    waiting_writer_count_db1--;
    assert(writer_count_db1 == 0);
    writer_count_db1++;
    pthread_mutex_unlock(&mutex_lock_db1);
    // USE (DO_TASK) for db1
    shared_data++;
    usleep(200000);
    printf("I wrote to db #1\n"); 
    // RELEASE for db1
    pthread_mutex_lock(&mutex_lock_db1);
    writer_count_db1--;
    assert(writer_count_db1 == 0);
    if (waiting_writer_count_db1 > 0) {
      pthread_cond_signal(&writers_cond); 
    }
    pthread_mutex_unlock(&mutex_lock_db1);
    usleep(500000); 

    // ACQUIRE for db2
    pthread_mutex_lock(&mutex_lock_db2);
    waiting_writer_count_db2++;
    while (writer_count_db2 > 0) {
      pthread_cond_wait(&writers_cond, &mutex_lock_db2);
    }
    waiting_writer_count_db2--;
    assert(writer_count_db2 == 0);
    writer_count_db2++;
    pthread_mutex_unlock(&mutex_lock_db2);
    // USE (DO_TASK) for db2
    shared_data++;
    usleep(200000);
    printf("I wrote to db #2\n"); 
    // RELEASE for db2
    pthread_mutex_lock(&mutex_lock_db2);
    writer_count_db2--;
    assert(writer_count_db2 == 0);
    if (waiting_writer_count_db2 > 0) {
      pthread_cond_signal(&writers_cond); 
    }
    pthread_mutex_unlock(&mutex_lock_db2);
    usleep(500000); 
  }
  return NULL;
}

int main() {
  pthread_t writers[NUM_WRITERS];
  int writer_ids[NUM_WRITERS];
  pthread_mutex_init(&mutex_lock_db1, NULL);
  pthread_mutex_init(&mutex_lock_db2, NULL);
  pthread_cond_init(&writers_cond, NULL);
  for (int i = 0; i < NUM_WRITERS; i++) {
    writer_ids[i] = i + 1;
    if (pthread_create(&writers[i], NULL, writer, &writer_ids[i]) != 0) {
      return 1;
    }
  }
  for (int i = 0; i < NUM_WRITERS; i++) {
    pthread_join(writers[i], NULL);
  }
  pthread_mutex_destroy(&mutex_lock_db1);
  pthread_mutex_destroy(&mutex_lock_db2);
  pthread_cond_destroy(&writers_cond);
  return 0;
}
