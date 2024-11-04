#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_t thread;

void * thread_doit(void *unused) {

    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    printf("\n*********Cond_wait called inside user********\n");
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[]) {

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&thread, NULL, &thread_doit, NULL);
    pthread_create(&thread, NULL, &thread_doit, NULL);

    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_join(thread, NULL);

    return 0;
}
