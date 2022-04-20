#include "GMAL.h"
#include "GMALWrappers.h"
#include "transitions/GMALThreadStart.h"
#include <typeinfo>
#include <typeindex>

#include <pthread.h>

//pthread_t main_thread;

pthread_mutex_t m1;
pthread_mutex_t m2;

void*
thread_helper_main(void *unused) {
    gmal_pthread_mutex_lock(&m2);
    gmal_pthread_mutex_lock(&m1);
    return NULL;
}

int
main(int argc, const char **argv)
{
    gmal_init();
    pthread_t helper;

    gmal_pthread_mutex_init(&m1, nullptr);
    gmal_pthread_mutex_init(&m2, nullptr);
    gmal_pthread_create(&helper, nullptr, &thread_helper_main, nullptr);
    gmal_pthread_mutex_lock(&m1);
    gmal_pthread_mutex_lock(&m2);
    return 0;
}