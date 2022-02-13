#include "dpor.h"

#define PTHREAD_SUCCESS (0)

static void*
dpor_schedule_main(void *unused)
{
    // TODO: DPOR algorithm here

    array_destroy(global_stack, NULL);

    return NULL;
}

// __attribute__((constructor))
void
dpor_init(void)
{
    // 1. Initialize the global state and transition stacks
    global_stack = array_create();

    // Spawn the dpor thread itself that runs the program

    pthread_t dpor_thread;
    if (pthread_create(&dpor_thread, NULL, &dpor_schedule_main, NULL) != PTHREAD_SUCCESS)
        return; // TODO: Fail properly here


}
