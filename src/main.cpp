#include "GMAL.h"
#include "GMALWrappers.h"
#include "transitions/GMALThreadStart.h"
#include <typeinfo>
#include <typeindex>

void*
thread_helper_main(void *unused) {
    return NULL;
}

struct Test {

};

using TypeInfoRef = std::reference_wrapper<const std::type_info>;

#include <iostream>
#include <type_traits>

int
main(int argc, const char **argv)
{
    gmal_init();

    pthread_t helper;


    gmal_pthread_create(&helper, nullptr, &thread_helper_main, nullptr);
    gmal_pthread_create(&helper, nullptr, &thread_helper_main, nullptr);

    gmal_exit_main_thread();
    return 0;
}