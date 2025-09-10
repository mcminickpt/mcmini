#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Shared variable
int shared_variable = 0;

// Semaphore to control access
sem_t sem_sync;

// Thread function for the writer
void *writer_thread(void *arg) {
    printf("Writer thread: Setting shared_variable to 10\n");
    shared_variable = 10;
    
    // Signal the reader that data is ready
    printf("Writer thread: Signaling the reader\n");
    sem_post(&sem_sync);
    
    return NULL;
}

// Thread function for the reader
void *reader_thread(void *arg) {
    printf("Reader thread: Waiting for the writer to signal...\n");

    // Wait for the writer to post the semaphore
    sem_wait(&sem_sync);

    printf("Reader thread: Signal received. Reading shared_variable: %d\n", shared_variable);

    return NULL;
}

int main() {
    pthread_t writer, reader;

    // Initialize the semaphore to 0. This means the reader will block initially.
    sem_init(&sem_sync, 0, 0);

    // Create the writer and reader threads
    pthread_create(&writer, NULL, writer_thread, NULL);
    pthread_create(&reader, NULL, reader_thread, NULL);

    // Wait for the threads to finish
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    // Destroy the semaphore
    sem_destroy(&sem_sync);

    return 0;
}
