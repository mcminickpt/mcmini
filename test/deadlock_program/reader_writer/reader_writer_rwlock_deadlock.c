#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_LOOP 2

pthread_rwlock_t rw;

int DEBUG = 0;

void *reader(void *unused) {
    for(int i=0; i< NUM_LOOP; i++) {
        pthread_rwlock_rdlock(&rw);
        if(DEBUG)
            printf("reader is reading\n");
        sleep(1);
        pthread_rwlock_unlock(&rw);
    }
    return NULL;
}

void *writer(void *unused) {
    for(int i=0; i< NUM_LOOP; i++) {
        pthread_rwlock_wrlock(&rw);
        if(DEBUG)
            printf("writer is writing\n");
        sleep(5);
        pthread_rwlock_unlock(&rw);
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

    pthread_rwlock_init(&rw, NULL);

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
