#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define DEFAULT_NUM_READERS 3
#define DEFAULT_NUM_WRITERS 3
#define DEFAULT_NUM_LOOP 2

pthread_mutex_t mutex;
pthread_cond_t cond;

int num_readers = 0;
int num_writers = 0;
int num_readers_waiting = 0;
int num_writers_waiting = 0;
int DEBUG = 0;

int read_condition() {
    return num_writers == 0;
}

int write_condition() {
    return (num_readers - num_readers_waiting) == 0 && (num_writers - num_writers_waiting) == 1;
}

void *reader(void *unused) {
    int num_loop = *((int*)unused);
    for(int i=0; i< num_loop; i++) {
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

void *writer(void *unused) {
    int num_loop = *((int*)unused);
    for(int i=0; i< num_loop; i++) {
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
    if(argc != 5){
        printf("Usage: %s NUM_READERS NUM_WRITERS NUM_LOOP DEBUG\n", argv[0]);
        return 1;
    }

    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);
    int num_loop    = atoi(argv[3]);
    DEBUG = atoi(argv[4]);

    pthread_t read_thread[num_readers];
    pthread_t write_thread[num_writers];
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    for (int i = 0; i < num_readers; i++) {
        pthread_create(&read_thread[i], NULL, reader, &num_loop);
    }
    for (int i = 0; i < num_writers; i++) {
        pthread_create(&write_thread[i], NULL, writer, &num_loop);
    }

    for (int i = 0; i < num_readers; i++) {
        pthread_join(read_thread[i], NULL);
    }
    for (int i = 0; i < num_writers; i++) {
        pthread_join(write_thread[i], NULL);
    }
    return 0;
}
