#include "pthread_wrappers.h"
#include "dpor.h"

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

void*
thread_helper_main(void *unused) {
//    dpor_pthread_mutex_lock(&mutex1);
//    dpor_pthread_mutex_lock(&mutex2);
//    dpor_pthread_mutex_unlock(&mutex2);
//    dpor_pthread_mutex_unlock(&mutex1);
    return NULL;
}

int
main(int argc, const char **argv) {

    // Normally won't exist in the real program
    dpor_init();

    pthread_t helper;

    dpor_pthread_mutex_init(&mutex1, NULL);
    dpor_pthread_mutex_init(&mutex2, NULL);
//    dpor_pthread_create(&helper, NULL, &thread_helper_main, NULL);
//
//    dpor_pthread_mutex_lock(&mutex2);
//    dpor_pthread_mutex_lock(&mutex1);
//    dpor_pthread_mutex_unlock(&mutex1);
//    dpor_pthread_mutex_unlock(&mutex2);
//
//    dpor_pthread_join(&helper, NULL);

    return 0;
}






