#include "GMAL.h"
#include "GMALWrappers.h"
#include "transitions/GMALThreadStart.h"
#include <typeinfo>
#include <typeindex>

#include <pthread.h>

pthread_t main_thread;

void*
thread_helper_main(void *unused) {
    gmal_pthread_join(main_thread, nullptr);
    return NULL;
}

#include <iostream>
#include <type_traits>

int
main(int argc, const char **argv)
{
    gmal_init();

    main_thread = pthread_self();

    pthread_t helper;

    gmal_pthread_create(&helper, nullptr, &thread_helper_main, nullptr);
    gmal_pthread_join(helper, nullptr);
//    gmal_pthread_create(&helper, nullptr, &thread_helper_main, nullptr);

    gmal_exit_main_thread();
    return 0;
}