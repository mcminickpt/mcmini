#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_LOOP 2

pthread_mutex_t rw, read_mutex;

int num_readers = 0;
int DEBUG = 0;

void *reader(void *notused) {
    for(int i=0; i< NUM_LOOP; i++) {
        pthread_mutex_lock(&read_mutex);
        num_readers++;
        if(num_readers==1)
            pthread_mutex_lock(&rw);
        pthread_mutex_unlock(&read_mutex);

        if (DEBUG) printf("reader is reading\n");

        pthread_mutex_lock(&read_mutex);
        num_readers--;
        if(num_readers==0)
            pthread_mutex_unlock(&rw);
        pthread_mutex_unlock(&read_mutex);
    }
    return NULL;
}

void *writer(void *notused) {
    for(int i=0; i< NUM_LOOP; i++) {
        pthread_mutex_lock(&rw);
        if (DEBUG) printf("writer is writing\n");

        pthread_mutex_unlock(&rw);
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
    pthread_mutex_init(&read_mutex, NULL);
    pthread_mutex_init(&rw, NULL);
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
