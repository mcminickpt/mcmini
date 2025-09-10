#include <pthread.h>
#include <stdio.h>

int counter = 0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void* thread_func(void* arg) {
    for (int i = 0; i < 1000; i++) {
        pthread_mutex_lock(&counter_mutex);
        counter++;  // SAFE: Protected by mutex
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

int main() {
    pthread_t thr1, thr2;

    pthread_create(&thr1, NULL, thread_func, NULL);
    pthread_create(&thr2, NULL, thread_func, NULL);

    pthread_join(thr1, NULL);
    pthread_join(thr2, NULL);

    printf("Final counter: %d\n", counter);
    return 0;
}

