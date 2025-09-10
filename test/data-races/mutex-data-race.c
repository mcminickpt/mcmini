#include <pthread.h>
#include <stdio.h>

int counter = 0;
pthread_mutex_t counter_mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t counter_mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread_func1(void* arg) {
    for (int i = 0; i < 1000; i++) {
        pthread_mutex_lock(&counter_mutex1);
        counter++;
        pthread_mutex_unlock(&counter_mutex1);
    }
    return NULL;
}

void* thread_func2(void* arg) {
    for (int i = 0; i < 1000; i++) {
        pthread_mutex_lock(&counter_mutex2);
        counter++;
        pthread_mutex_unlock(&counter_mutex2);
    }
    return NULL;
}

int main() {
    pthread_t thr1, thr2;

    pthread_create(&thr1, NULL, thread_func1, NULL);
    pthread_create(&thr2, NULL, thread_func2, NULL);

    pthread_join(thr1, NULL);
    pthread_join(thr2, NULL);

    printf("Final counter: %d\n", counter);
    return 0;
}

