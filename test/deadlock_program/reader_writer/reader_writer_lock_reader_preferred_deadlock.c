#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t rw, read;

int num_readers = 0;
int DEBUG = 0;

void *reader(void *notused) {
    for(int i = 0; i < notused; i++) {
        pthread_mutex_lock(&read);
        num_readers++;
        if(num_readers == 1)
            pthread_mutex_lock(&rw);
        pthread_mutex_unlock(&read);

        if(DEBUG) printf("reader is reading\n");

        pthread_mutex_lock(&read);
        num_readers--;
        if(num_readers == 0)
            pthread_mutex_unlock(&rw);
        pthread_mutex_unlock(&read);
    }
    return NULL;
}

void *writer(void *notused) {
    for(int i = 0; i < notused; i++) {
        pthread_mutex_lock(&rw);

        if(DEBUG) printf("writer is writing\n");

        pthread_mutex_unlock(&rw);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if(argc != 5){
        printf("Usage: %s NUM_READERS NUM_WRITERS NUM_LOOP DEBUG\n", argv[0]);
        return 1;
    }

    int NUM_READERS = atoi(argv[1]);
    int NUM_WRITERS = atoi(argv[2]);
    int NUM_LOOP = atoi(argv[3]);
    DEBUG = atoi(argv[4]);

    pthread_t read_thread[NUM_READERS];
    pthread_t write_thread[NUM_WRITERS];

    pthread_mutex_init(&read, NULL);
    pthread_mutex_init(&rw, NULL);
    
    int i;
    for (i = 0; i < NUM_READERS; i++) {
        pthread_create(&read_thread[i], NULL, reader, (void*)NUM_LOOP);
    }
    for (i = 0; i < NUM_WRITERS; i++) {
        pthread_create(&write_thread[i], NULL, writer, (void*)NUM_LOOP);
    }

    for (i = 0; i < NUM_READERS; i++) {
        pthread_join(read_thread[i], NULL);
    }
    for (i = 0; i < NUM_WRITERS; i++) {
        pthread_join(write_thread[i], NULL);
    }
    return 0;
}
