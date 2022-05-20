#include <stdio.h>
#include <unistd.h>
#include "MCMINI.h"
#include "MCMINIWrappers.h"

#define NUM_READERS 3
#define NUM_WRITERS 3
#define NUM_LOOP 2

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
    return num_readers == 0 && (num_writers - num_writers_waiting) == 1; // This thread is the 1 writer
}

void *reader(void *notused) {
    for(int i=0; i< NUM_LOOP; i++) {
        // acquire resource
        gmal_pthread_mutex_lock(&mutex);
        num_readers++;
        while (! read_condition()) {
            num_readers_waiting++;
            gmal_pthread_cond_wait(&cond, &mutex); // wait on cond
            num_readers_waiting--;
        }
        gmal_pthread_mutex_unlock(&mutex);
        // use resource (we fake this by sleeping)
        printf("reader is reading\n");
        sleep(1);
        // release resource
        gmal_pthread_mutex_lock(&mutex);
        num_readers--;
        gmal_pthread_cond_broadcast(&cond); // wake up everyone and let them try again
        // NOTE: For efficiency, once could consider multiple calls to
        //   pthread_cond_signal above, and wake up only as many threads as needed.
        //   However, carefully check your code for correctness if you try that.
        gmal_pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}

void *writer(void *notused) {
    for(int i=0; i< NUM_LOOP; i++) {
        // acquire resource
        gmal_pthread_mutex_lock(&mutex);
        num_writers++;
        while (! write_condition()) {
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
        gmal_pthread_cond_broadcast(&cond); // wake up everyone and let them try again
        gmal_pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}

int main() {
    pthread_t read_thread[NUM_READERS];
    pthread_t write_thread[NUM_WRITERS];
    int i;

    gmal_init();
    gmal_pthread_mutex_init(&mutex, nullptr);
    gmal_pthread_cond_init(&cond, nullptr);

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
        gmal_pthread_join(write_thread[i], NULL);
    }
    return 0;
}
