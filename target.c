#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 4
#define NUM_INCREMENTS 100000

int counter = 0;

void *increment_counter(void *arg) {
    for (int i = 0; i < NUM_INCREMENTS; i++) {
        counter++; // Critical section
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, increment_counter, NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final counter value: %d\n", counter);

    return 0;
}

