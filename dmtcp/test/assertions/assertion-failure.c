#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h> // For sleep

// Global variables to be set by each thread
int global_var_thread1 = 0;
int global_var_thread2 = 0;
pthread_mutex_t lock_global_variables;

// Function executed by thread 1
void *thread_function1(void *arg) {
    printf("Thread 1: Starting...\n");

    // Introduce a small delay to increase the chance of the bug appearing
    usleep(100); // Sleep for 100 microseconds

    // BUG: Check without proper synchronization
    pthread_mutex_lock(&lock_global_variables);
    int var_thread2 = global_var_thread2;
    pthread_mutex_unlock(&lock_global_variables);

    if (!var_thread2) {
        printf("Thread 1: global_var_thread2 is 0, setting global_var_thread1 to 1.\n");
        pthread_mutex_lock(&lock_global_variables);
        global_var_thread1 = 1;
        pthread_mutex_unlock(&lock_global_variables);
    } else {
        printf("Thread 1: global_var_thread2 is already 1.\n");
    }

    printf("Thread 1: Finished. global_var_thread1 = %s\n", global_var_thread1 ? "1" : "0");
    return NULL;
}

// Function executed by thread 2
void *thread_function2(void *arg) {
    printf("Thread 2: Starting...\n");

    // Introduce a small delay to increase the chance of the bug appearing
    usleep(100); // Sleep for 100 microseconds

    // BUG: Check without proper synchronization
    pthread_mutex_lock(&lock_global_variables);
    int var_thread1 = global_var_thread1;
    pthread_mutex_unlock(&lock_global_variables);

    if (!var_thread1) {
        printf("Thread 2: global_var_thread1 is 0, setting global_var_thread2 to 1.\n");
        pthread_mutex_lock(&lock_global_variables);
        global_var_thread2 = 1;
        pthread_mutex_unlock(&lock_global_variables);
    } else {
        printf("Thread 2: global_var_thread1 is already 1.\n");
    }

    printf("Thread 2: Finished. global_var_thread2 = %s\n", global_var_thread2 ? "1" : "0");
    return NULL;
}

int main() {
    pthread_t tid1, tid2;

    printf("Main: Creating threads...\n");

    // Create thread 1
    if (pthread_create(&tid1, NULL, thread_function1, NULL) != 0) {
        perror("Failed to create thread 1");
        return 1;
    }

    // Create thread 2
    if (pthread_create(&tid2, NULL, thread_function2, NULL) != 0) {
        perror("Failed to create thread 2");
        return 1;
    }

    // Wait for threads to finish
    if (pthread_join(tid1, NULL) != 0) {
        perror("Failed to join thread 1");
        return 1;
    }
    if (pthread_join(tid2, NULL) != 0) {
        perror("Failed to join thread 2");
        return 1;
    }

    assert(global_var_thread1 ^ global_var_thread2);

    printf("\nMain: Both threads finished.\n");
    printf("Final global_var_thread1: %s\n", global_var_thread1 ? "1" : "0");
    printf("Final global_var_thread2: %s\n", global_var_thread2 ? "1" : "0");

    return 0;
}
