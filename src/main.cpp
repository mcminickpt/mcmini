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
    auto a = std::type_index(typeid(Test));


    gmal_init();

    pthread_t helper;

    GMALSharedTransition test = GMALSharedTransition(typeid(GMALThreadStart));
    memcpy(shmTransitionData, &test, sizeof(GMALSharedTransition));

    thread_await_gmal_scheduler();



//    gmal_pthread_create(&helper, nullptr, &thread_helper_main, nullptr);
    return 0;
}