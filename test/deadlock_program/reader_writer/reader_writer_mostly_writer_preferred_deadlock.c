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

int read_condition(int NUM_READERS) {
  return num_writers == 0 || ((num_writers - num_writers_waiting) == 0 && num_readers_waiting == NUM_READERS - 1);
}

int write_condition(int NUM_READERS) {
  return num_readers_waiting != NUM_READERS && (num_readers - num_readers_waiting) == 0 && (num_writers - num_writers_waiting) == 1;
}

void *reader(void *NUM_LOOP) {
    int num_loop = *(int *)NUM_LOOP;
    for(int i=0; i < num_loop; i++) {
        pthread_mutex_lock(&mutex);
        num_readers++;
        while (! read_condition(num_readers + num_readers_waiting)) {
            num_readers_waiting++;
            pthread_cond_wait(&cond, &mutex);
            num_readers_waiting--;
        }
        pthread_mutex_unlock(&mutex);

        if(DEBUG) printf("reader is reading\n");

        pthread_mutex_lock(&mutex);
        num_readers--;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *writer(void *NUM_LOOP) {
    int num_loop = *(int *)NUM_LOOP;
    for(int i=0; i < num_loop; i++) {
        pthread_mutex_lock(&mutex);
        num_writers++;
        while (! write_condition(num_readers + num_readers_waiting)) {
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

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    int i;

    for (i = 0; i < NUM_READERS; i++) {
        pthread_create(&read_thread[i], NULL, reader, &NUM_LOOP);
    }
    for (i = 0; i < NUM_WRITERS; i++) {
        pthread_create(&write_thread[i], NULL, writer, &NUM_LOOP);
    }

    for (i = 0; i < NUM_READERS; i++) {
        pthread_join(read_thread[i], NULL);
    }
    for (i = 0; i < NUM_WRITERS; i++) {
        pthread_join(write_thread[i], NULL);
    }
    return 0;
}
