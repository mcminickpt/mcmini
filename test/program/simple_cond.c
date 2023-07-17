#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

pthread_mutex_t mutex, mutex_start;
pthread_cond_t cond;
pthread_t thread;

void * thread_doit(void *unused)
{
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex_start);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[])
{
    if (argc > 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    if(pthread_mutex_init(&mutex, NULL) != 0) {
        printf("Failed to initialize mutex\n");
        return EXIT_FAILURE;
    }
    if(pthread_mutex_init(&mutex_start, NULL) != 0) {
        printf("Failed to initialize mutex_start\n");
        return EXIT_FAILURE;
    }
    if(pthread_cond_init(&cond, NULL) != 0) {
        printf("Failed to initialize cond\n");
        return EXIT_FAILURE;
    }

    pthread_mutex_lock(&mutex_start);
    if(pthread_create(&thread, NULL, &thread_doit, NULL) != 0) {
        printf("Failed to create thread\n");
        return EXIT_FAILURE;
    }

    // Wait for the thread to get to the condition variable
    pthread_mutex_lock(&mutex_start);
    pthread_mutex_unlock(&mutex_start);

    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    if(pthread_join(thread, NULL) != 0) {
        printf("Failed to join thread\n");
        return EXIT_FAILURE;
    }

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_start);
    pthread_cond_destroy(&cond);

    return EXIT_SUCCESS;
}
