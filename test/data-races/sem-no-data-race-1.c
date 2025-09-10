#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

int counter = 0;
sem_t counter_sem;

void* thread_func(void* arg) {
    for (int i = 0; i < 1000; i++) {
        sem_wait(&counter_sem);  // Acquire semaphore
        counter++;               // Critical section
        sem_post(&counter_sem);  // Release semaphore
    }
    return NULL;
}

int main() {
    pthread_t thr1, thr2;

    // Initialize semaphore to 1 (binary semaphore)
    sem_init(&counter_sem, 0, 1);

    pthread_create(&thr1, NULL, thread_func, NULL);
    pthread_create(&thr2, NULL, thread_func, NULL);

    pthread_join(thr1, NULL);
    pthread_join(thr2, NULL);

    printf("Final counter: %d\n", counter);

    // Clean up semaphore
    sem_destroy(&counter_sem);

    return 0;
}

