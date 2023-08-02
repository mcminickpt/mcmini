#include "rwwlock.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_LOOP 2

pthread_rwwlock_t rw;

int DEBUG = 0;

void *reader(void *notused) {
    for (int i = 0; i < NUM_LOOP; i++) {
        pthread_rwwlock_rdlock(&rw);

        if (DEBUG) printf("reader is reading\n");

        pthread_rwwlock_unlock(&rw);
    }
    return NULL;
}

void *writer1(void *notused) {
    for (int i = 0; i < NUM_LOOP; i++) {
        pthread_rwwlock_wr1lock(&rw);

        if (DEBUG) printf("writer 1 is writing\n");

        pthread_rwwlock_unlock(&rw);
    }
    return NULL;
}

void *writer2(void *notused) {
    for (int i = 0; i < NUM_LOOP; i++) {
        pthread_rwwlock_wr2lock(&rw);

        if (DEBUG) printf("writer 2 is writing\n");

        pthread_rwwlock_unlock(&rw);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if(argc != 5){
        printf("Usage: %s NUM_READERS NUM_TYPE1_WRITERS NUM_TYPE2_WRITERS DEBUG\n", argv[0]);
        return 1;
    }

    int NUM_READERS = atoi(argv[1]);
    int NUM_TYPE1_WRITERS = atoi(argv[2]);
    int NUM_TYPE2_WRITERS = atoi(argv[3]);
    DEBUG = atoi(argv[4]);

    pthread_t read_thread[NUM_READERS];
    pthread_t writer1_thread[NUM_TYPE1_WRITERS];
    pthread_t writer2_thread[NUM_TYPE2_WRITERS];

    pthread_rwwlock_init(&rw);

    int i;
    for (i = 0; i < NUM_READERS; i++) {
        pthread_create(&read_thread[i], NULL, reader, NULL);
    }
    for (i = 0; i < NUM_TYPE1_WRITERS; i++) {
        pthread_create(&writer1_thread[i], NULL, writer1, NULL);
    }
    for (i = 0; i < NUM_TYPE2_WRITERS; i++) {
        pthread_create(&writer2_thread[i], NULL, writer2, NULL);
    }

    for (i = 0; i < NUM_READERS; i++) {
        pthread_join(read_thread[i], NULL);
    }
    for (i = 0; i < NUM_TYPE1_WRITERS; i++) {
        pthread_join(writer1_thread[i], NULL);
    }
    for (i = 0; i < NUM_TYPE2_WRITERS; i++) {
        pthread_join(writer2_thread[i], NULL);
    }
    return 0;
}
