#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex;
pthread_cond_t cond;

int num_readers = 0;
int num_writers = 0;
int num_readers_waiting = 0;
int num_writers_waiting = 0;

int DEBUG = 0;

int read_condition() {
    return (num_writers - num_writers_waiting) == 0;
}

int write_condition() {
    return num_readers == 0 && (num_writers - num_writers_waiting) == 1; // This thread is the 1 writer
}

void *reader(void *notused) {
    for(int i=0; i< 2; i++) {
        pthread_mutex_lock(&mutex);
        num_readers++;
        while (! read_condition()) {
            num_readers_waiting++;
            pthread_cond_wait(&cond, &mutex);
            num_readers_waiting--;
        }
        pthread_mutex_unlock(&mutex);
        if (DEBUG) printf("reader is reading\n");
        pthread_mutex_lock(&mutex);
        num_readers--;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *writer(void *notused) {
    for(int i=0; i< 2; i++) {
        pthread_mutex_lock(&mutex);
        num_writers++;
        while (! write_condition()) {
            num_writers_waiting++;
            pthread_cond_wait(&cond, &mutex);
            num_writers_waiting--;
        }
        pthread_mutex_unlock(&mutex);
        if (DEBUG) printf("writer is writing\n");
        pthread_mutex_lock(&mutex);
        num_writers--;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if(argc != 4){
        printf("Usage: %s NUM_READERS NUM_WRITERS DEBUG\n", argv[0]);
        return 1;
    }

    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);
    DEBUG = atoi(argv[3]);

    pthread_t read_thread[num_readers];
    pthread_t write_thread[num_writers];
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    int i;
    for (i = 0; i < num_readers; i++) {
        pthread_create(&read_thread[i], NULL, reader, NULL);
    }
    for (i = 0; i < num_writers; i++) {
        pthread_create(&write_thread[i], NULL, writer, NULL);
    }

    for (i = 0; i < num_readers; i++) {
        pthread_join(read_thread[i], NULL);
    }
    for (i = 0; i < num_writers; i++) {
        pthread_join(write_thread[i], NULL);
    }
    return 0;
}
