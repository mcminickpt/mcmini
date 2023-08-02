#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_LOOP 2

pthread_mutex_t mutex;
pthread_cond_t read_cond, write_cond;

int num_readers = 0;
int num_writers = 0;
int num_read_requests = 0;
int num_write_requests = 0;
int DEBUG = 0;

void *reader(void *unused) {
    for(int i=0; i< NUM_LOOP; i++) {
        pthread_mutex_lock(&mutex);
        num_read_requests++;
        while (num_writers > 0) {
            pthread_cond_wait(&read_cond, &mutex);
        }
        num_read_requests--;
        num_readers++;
        pthread_mutex_unlock(&mutex);

        if (DEBUG) printf("reader is reading\n");

        pthread_mutex_lock(&mutex);
        num_readers--;
        if(num_readers == 0) pthread_cond_signal(&write_cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *writer(void *unused) {
    for( int i=0; i<NUM_LOOP; i++){
        pthread_mutex_lock(&mutex);
        num_write_requests++;
        while (num_readers > 0 || num_writers > 0) {
            pthread_cond_wait(&write_cond, &mutex);
        }
        num_write_requests--;
        num_writers++;
        pthread_mutex_unlock(&mutex);

        if (DEBUG) printf("writer is writing\n");

        pthread_mutex_lock(&mutex);
        num_writers--;
        pthread_cond_signal(&write_cond);
        if(num_read_requests > 0) pthread_cond_broadcast(&read_cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
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
    pthread_cond_init(&read_cond, NULL);
    pthread_cond_init(&write_cond, NULL);

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
