#include <stdio.h>
#include <pthread.h>
#include "GMAL.h"
#include "GMALWrappers.h"

#define NUM_READERS 3
#define NUM_WRITERS 3

pthread_mutex_t mutex;
pthread_cond_t cond;

int num_readers = 0;
int num_writers = 0;
int num_readers_waiting = 0;
int num_writers_waiting = 0;

int read_condition() {
  // (num_writers - num_writers_waiting) is the number of _active_ writers.
  return (num_writers - num_writers_waiting) == 0;
}

int write_condition() {
  return (num_readers - num_readers_waiting) == 0 && (num_writers - num_writers_waiting) == 1; // This thread is the 1 writer
}

void *reader(void *notused) {
  while (1) {
    // acquire resource
    pthread_mutex_lock(&mutex);
    num_readers++;
    while (! read_condition()) {
      num_readers_waiting++;
      pthread_cond_wait(&cond, &mutex); // wait on cond
      num_readers_waiting--;
    }
    pthread_mutex_unlock(&mutex);
    // use resource (we fake this by sleeping)
    printf("reader is reading\n");
    sleep(1);
    // release resource
    pthread_mutex_lock(&mutex);
    num_readers--;
    pthread_cond_broadcast(&cond); // wake up everyone and let them try again
    // NOTE: For efficiency, once could consider multiple calls to
    //   pthread_cond_signal above, and wake up only as many threads as needed.
    //   However, carefully check your code for correctness if you try that.
    pthread_mutex_unlock(&mutex);
  }
}

void *writer(void *notused) {
  while (1) {
    // acquire resource
    pthread_mutex_lock(&mutex);
    num_writers++;
    while (! write_condition()) {
      num_writers_waiting++;
      pthread_cond_wait(&cond, &mutex); // wait on cond
      num_writers_waiting--;
    }
    pthread_mutex_unlock(&mutex);
    // use resource (we fake this by sleeping)
    printf("writer is writing\n");
    sleep(5);
    // release resource
    pthread_mutex_lock(&mutex);
    num_writers--;
    pthread_cond_broadcast(&cond); // wake up everyone and let them try again
    pthread_mutex_unlock(&mutex);
  }
}

int main() {

    pthread_t read_thread[NUM_READERS];
    pthread_t write_thread[NUM_WRITERS];
    int i;

    for (i = 0; i < NUM_READERS; i++) {
      gmal_pthread_create(&read_thread[i], NULL, reader, NULL);
    }
    for (i = 0; i < NUM_WRITERS; i++) {
      gmal_pthread_create(&write_thread[i], NULL, writer, NULL);
    }

    for (i = 0; i < NUM_READERS; i++) {
      gmal_pthread_join(read_thread[i], NULL);
    }
    for (i = 0; i < NUM_WRITERS; i++) {
      pthread_join(write_thread[i], NULL);
    }
    return 0;
}
