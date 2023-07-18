#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

int ticket_number = 0;
int next_ticket_to_be_served = 0;

pthread_mutex_t mutex;
pthread_cond_t cond;

int num_readers = 0;
int num_writers = 0;
int num_readers_waiting = 0;
int num_writers_waiting = 0;

int DEBUG = 0;

int read_condition(int reader_ticket_number) {
    return num_writers == 0 && reader_ticket_number == next_ticket_to_be_served;
}

int write_condition(int writer_ticket_number) {
    return (num_readers - num_readers_waiting) == 0 && (num_writers - num_writers_waiting) == 1
    && writer_ticket_number == next_ticket_to_be_served;
}

void *reader(void *notused) {
    while (1) {
        pthread_mutex_lock(&mutex);
        int local_reader_ticket_number = ticket_number;
        ticket_number++;
        num_readers++;
        while (! read_condition(local_reader_ticket_number)) {
            num_readers_waiting++;
            pthread_cond_wait(&cond, &mutex);
            num_readers_waiting--;
        }
        next_ticket_to_be_served++;
        pthread_mutex_unlock(&mutex);
        if(DEBUG) printf("reader is reading\n");
        pthread_mutex_lock(&mutex);
        num_readers--;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

void *writer(void *notused) {
    while (1) {
        pthread_mutex_lock(&mutex);
        int local_writer_ticket_number = ticket_number;
        ticket_number++;
        num_writers++;
        while (! write_condition(local_writer_ticket_number)) {
            num_writers_waiting++;
            pthread_cond_wait(&cond, &mutex);
            num_writers_waiting--;
        }
        pthread_mutex_unlock(&mutex);
        if(DEBUG) printf("writer is writing\n");
        pthread_mutex_lock(&mutex);
        num_writers--;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char* argv[]) {
    if(argc != 4){
        printf("Usage: %s NUM_READERS NUM_WRITERS DEBUG\n", argv[0]);
        return 1;
    }

    int NUM_READERS = atoi(argv[1]);
    int NUM_WRITERS = atoi(argv[2]);
    DEBUG = atoi(argv[3]);

    pthread_t read_thread[NUM_READERS];
    pthread_t write_thread[NUM_WRITERS];

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    int i;
    for (i = 0; i < NUM_READERS; i++) {
        pthread_create(&read_thread[i], NULL, reader, NULL);
    }
    for (i = 0; i < NUM_WRITERS; i++) {
        pthread_create(&write_thread[i], NULL, writer, NULL);
    }

    for (i = 0; i < NUM_READERS; i++) {
        pthread_join(read_thread[i], NULL);
    }
    for (i = 0; i < NUM_WRITERS; i++) {
        pthread_join(write_thread[i], NULL);
    }
    return 0;
}
