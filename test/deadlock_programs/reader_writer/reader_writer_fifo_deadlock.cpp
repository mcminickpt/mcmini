#include <stdio.h>
#include <unistd.h>
#include "GMAL.h"
#include "GMALWrappers.h"

#define NUM_READERS 5
#define NUM_WRITERS 2
#define NUM_LOOP 2

int ticket_number = 0;
int next_ticket_to_be_served = 0;

pthread_mutex_t mutex;
pthread_cond_t cond;

int num_readers = 0;
int num_writers = 0;
int num_readers_waiting = 0;
int num_writers_waiting = 0;

int read_condition(int reader_ticket_number) {
  return num_writers == 0 && reader_ticket_number==next_ticket_to_be_served;
}

int write_condition(int writer_ticket_number){
  return (num_readers - num_readers_waiting) == 0 && (num_writers - num_writers_waiting) == 1
    && writer_ticket_number==next_ticket_to_be_served;
}


void *reader(void *notused) {
  while (1) {
    // acquire resource
    gmal_pthread_mutex_lock(&mutex);
    int local_reader_ticket_number = ticket_number;
    ticket_number++;
    num_readers++;
    while (! read_condition(local_reader_ticket_number)) {
      num_readers_waiting++;
      gmal_pthread_cond_wait(&cond, &mutex); // wait on cond
      num_readers_waiting--;
    }
    next_ticket_to_be_served++;
    gmal_pthread_mutex_unlock(&mutex);
    // use resource (we fake this by sleeping)
    printf("reader is reading\n");
    sleep(1);
    // release resource
    gmal_pthread_mutex_lock(&mutex);
    num_readers--;
    gmal_pthread_cond_broadcast(&cond); // wake up everyone and let them try again
    gmal_pthread_mutex_unlock(&mutex);
  }
}

void *writer(void *notused) {
  while (1) {
    // acquire resource
    gmal_pthread_mutex_lock(&mutex);
    int local_writer_ticket_number = ticket_number;
    ticket_number++;
    num_writers++;
    while (!write_condition(local_writer_ticket_number)) {
      num_writers_waiting++;
      gmal_pthread_cond_wait(&cond, &mutex); // wait on cond
      num_writers_waiting--;
    }
    gmal_pthread_mutex_unlock(&mutex);
    // use resource (we fake this by sleeping)
    printf("writer is writing\n");
    sleep(5);
    // release resource
    gmal_pthread_mutex_lock(&mutex);
    num_writers--;
    gmal_pthread_cond_broadcast(&cond);
    gmal_pthread_mutex_unlock(&mutex);
  }
}

int main() {
    pthread_t read_thread[NUM_READERS];
    pthread_t write_thread[NUM_WRITERS];

    gmal_init();
    gmal_pthread_mutex_init(&mutex, nullptr);
    gmal_pthread_cond_init(&cond, nullptr);

    int i;

    for (i = 0; i < NUM_READERS; i++) {
        gmal_pthread_create(&read_thread[i], nullptr, reader, nullptr);
    }
    for (i = 0; i < NUM_WRITERS; i++) {
        gmal_pthread_create(&write_thread[i], nullptr, writer, nullptr);
    }

    for (i = 0; i < NUM_READERS; i++) {
        gmal_pthread_join(read_thread[i], nullptr);
    }
    for (i = 0; i < NUM_WRITERS; i++) {
        gmal_pthread_join(write_thread[i], nullptr);
    }
    return 0;
}
