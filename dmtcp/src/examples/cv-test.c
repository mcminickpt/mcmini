#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_CONSUMERS 3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int data_ready = 0;
int stop_flag = 0;
int destroy_cv = 0;  // Flag to control CV destruction

// Consumer threads wait for data
void* consumer(void* arg) {
    int id = *(int*)arg;
    
    printf("Consumer %d: Started\n", id);
    sleep(5);  // Give time for checkpoint
    
    pthread_mutex_lock(&mutex);
    
    // Wait for data to be ready
    while (!data_ready && !stop_flag) {
        printf("Consumer %d: Waiting for data...\n", id);
        pthread_cond_wait(&cond, &mutex);
        printf("Consumer %d: Woke up\n", id);
    }
    
    if (data_ready) {
        printf("Consumer %d: Processing data\n", id);
        sleep(1);  // Simulate processing
    } else {
        printf("Consumer %d: Stopped without data\n", id);
    }
    
    pthread_mutex_unlock(&mutex);
    printf("Consumer %d: Finished\n", id);
    return NULL;
}

// Producer broadcasts to all consumers
void* producer(void* arg) {
    printf("Producer: Started\n");
    sleep(5);  // Give time for consumers to start waiting
    
    pthread_mutex_lock(&mutex);
    data_ready = 1;
    printf("Producer: Data is ready, broadcasting to all consumers\n");
    pthread_cond_broadcast(&cond);  // Wake all consumers
    pthread_mutex_unlock(&mutex);
    
    printf("Producer: Finished\n");
    return NULL;
}

// Cleanup thread might destroy CV prematurely
void* cleanup(void* arg) {
    printf("Cleanup: Started\n");
    sleep(5);  // Sleep to allow consumers to start waiting
    
    if (destroy_cv) {
        // Potential bug: Destroy CV while threads are waiting
        printf("Cleanup: Destroying condition variable prematurely!\n");
        pthread_cond_destroy(&cond);  // Problematic if threads are waiting
        printf("Cleanup: Destroyed\n");
    } else {
        printf("Cleanup: Decided not to destroy CV\n");
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t consumers[NUM_CONSUMERS];
    pthread_t prod, clean;
    int ids[NUM_CONSUMERS];
    
    // Allow command-line control over deadlock scenario
    if (argc > 1 && atoi(argv[1]) != 0) {
        destroy_cv = 1;
        printf("Running with CV destruction (deadlock possible)\n");
    } else {
        printf("Running without CV destruction (normal operation)\n");
    }
    
    // Create consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer, &ids[i]);
    }
    
    // Create producer and cleanup threads
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&clean, NULL, cleanup, NULL);
    
    // Allow time for checkpoint
    printf("Main: Sleeping to allow operations to proceed\n");
    sleep(5);
    
    // Join all threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    pthread_join(prod, NULL);
    pthread_join(clean, NULL);
    
    sleep(5);
    // Clean up if needed
    if (!destroy_cv) {
        pthread_cond_destroy(&cond);
    }
    pthread_mutex_destroy(&mutex);
    
    printf("Main: Program completed\n");
    return 0;
}