#include "dpor.h"

#define PTHREAD_SUCCESS (0)
#define MAX_DEPTH (1)

static void*
dpor_scheduler_main(void *unused)
{
    dpor_shared = dpor_context_create();
    sem_post(&dpor_init_sem);
    sem_wait(&main_thread_init_sem);

    // 1. Create the initial state
    state_stack_item_ref initial_stack_item = state_stack_item_create();
    if (!initial_stack_item)
        goto dpor_exit;

    {
        transition_ref main_thread_transition = create_thread_start_transition(dpor_shared->main_thread->thread);
        array_append(initial_stack_item->state->transitions, main_thread_transition);
    }

    int depth = 0;
    state_stack_item_ref s = initial_stack_item;
    transition_ref t = shared_state_first_enabled_transition(s->state);
    while (depth++ <= MAX_DEPTH && t != NULL) {
        // Push s and t onto the stack
        array_append(dpor_shared->state_stack, s);
        array_append(dpor_shared->transition_stack, t);

        // Let the program run to the next visible operation
        dpor_run(dpor_shared, t->thread);

        // Pop the next state off of the stack (pushed onto
        // the stack by the appropriate pthread wrapper call
        s = array_remove_last(dpor_shared->state_stack);

        // Update the backtrack sets
        dynamically_update_backtrack_sets(s);
    }

dpor_exit:
    dpor_context_destroy(dpor_shared);
    return NULL;
}

// __attribute__((constructor))
void
dpor_init(void)
{
    sem_init(&dpor_init_sem, 0, 0);
    sem_init(&main_thread_init_sem, 0, 0);

    // Spawn the dpor thread itself that runs the program
    pthread_t dpor_thread;
    if (pthread_create(&dpor_thread, NULL, &dpor_scheduler_main, NULL) != PTHREAD_SUCCESS)
        abort(); // TODO: Fail properly here

    // Wait for DPOR to initialize itself
    sem_wait(&dpor_init_sem);
    dpor_register_main_thread(dpor_shared, pthread_self());
    sem_post(&main_thread_init_sem);

    // Wait AGAIN for scheduling to start
    thread_await_dpor_scheduler(dpor_shared);
}

void
dpor_register_main_thread(dpor_context_ref context, pthread_t main_thread)
{
    dpor_register_thread(context, main_thread);
    context->main_thread = array_get_first(dpor_shared->thread_table);
}

void
dpor_register_thread(dpor_context_ref context, pthread_t pthread)
{
    othread_ref othread = othread_pthread_wrap(pthread);
    array_append(context->thread_table, othread);
}

void
dpor_run(dpor_context_ref context, thread_ref thread) {
    if (!thread) return; // ERROR // FIXME: ERROR


    // FIXME: What should happen if the thread doesn't exist
}