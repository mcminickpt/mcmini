#include "GMAL.h"
#include "GMALWrappers.h"

void*
thread_helper_main(void *unused) {
    return NULL;
}


int
main(int argc, const char **argv)
{
    gmal_init();

    pthread_t helper;
//    gmal_pthread_create(&helper, nullptr, &thread_helper_main, nullptr);
    return 0;
}