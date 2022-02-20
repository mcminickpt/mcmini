#include "dpor.h"

#define PTHREAD_SUCCESS (0)
#define MAX_DEPTH (1)

static sem_t dpor_init_sem;
static sem_t main_thread_init_sem;

MEMORY_ALLOC_DEF_DECL(dpor_context);

dpor_context_ref
dpor_context_create(void)
{
    dpor_context_ref dpor = dpor_context_alloc();
    if (!dpor) return NULL;
    dpor->transition_stack = array_create();
    dpor->state_stack = array_create();
    dpor->main_thread = NULL;
    dpor->thread_table = array_create();
    return dpor;
}

dpor_context_ref
dpor_context_copy(dpor_context_refc context)
{

}

void
dpor_context_destroy(dpor_context_ref dpor)
{
    if (!dpor) return;
    array_destroy(dpor->transition_stack, (free_function) transition_destroy);
    array_destroy(dpor->state_stack, (free_function) state_stack_item_destroy);
    array_destroy(dpor->thread_table, (free_function) othread_destroy);
    free(dpor);
}


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

    transition_ref main_thread_transition = create_thread_start_transition(dpor_shared->main_thread->thread);
    array_append(initial_stack_item->state->transitions, main_thread_transition);

    int depth = 0;
    state_stack_item_ref s = initial_stack_item;
    transition_ref t = main_thread_transition;
    while (depth++ <= MAX_DEPTH && t != NULL) {
        // Push s and t onto the stack
        array_append(dpor_shared->state_stack, s);
        array_append(dpor_shared->transition_stack, t);
        dpor_run(dpor_shared, t->thread);
        // Pop the next state off of the stack (pushed onto
        // the stack by the appropriate pthread wrapper call)
        s = array_remove_last(dpor_shared->state_stack);
        dynamically_update_backtrack_sets(dpor_shared, s);
        t = shared_state_get_first_enabled_transition(s->state);
    }

    // Do actual backtracking

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
    if (!thread) return; // ERROR // FIXME: how should this error be handled
    othread_ref othread = NULL; // TODO: Find the appropriate othread for the thread passed in
    sem_post(&othread->pthread_sem);
    sem_wait(&othread->dpor_scheduler_sem);
}

void
thread_await_dpor_scheduler(dpor_context_ref context) {
    if (!context) abort(); // TODO: How should we respond to an error here

    othread_ref othread = NULL;
    {
        thread_ref new_thread = thread_self();
        othread = NULL; // TODO: Fetch the appropriate thread entry for the new_thread
        thread_destroy(new_thread);
    }
    sem_post(&othread->dpor_scheduler_sem);
    sem_wait(&othread->pthread_sem);
}

static int
latest_dependent_coenabled_transition_index(dpor_context_ref context, transition_ref transition)
{
    if (!context || !transition) return -1;
    if (array_is_empty(context->transition_stack)) return -1;

    uint32_t transition_stack_size = array_count(context->transition_stack);
    for (uint32_t i = transition_stack_size - 1; i >= 0; i--) {
        transition_ref transition_i = array_get(context->transition_stack, i);
        if (transitions_dependent(transition_i, transition) && transitions_coenabled(transition_i, transition)) {
            return (int)i;
        }
    }
    return -1;
}

static array_ref
compute_set_E(dpor_context_ref context,
              transition_array_ref enabled_transitions,
              thread_ref thread,
              int state_stack_index)
{
    if (!enabled_transitions || !thread) return NULL;
    thread_array_ref E = array_create();
    if (!E) return NULL;
    uint32_t nts = array_count(enabled_transitions);
    for (uint32_t i = 0; i < nts; i++) {
        transition_ref trans = array_get(enabled_transitions, i);
        if (threads_equal(thread, trans->thread)) {
            thread_ref new_thread = thread_copy(thread);
            array_append(E, new_thread);
        }

        // Look through the transition stack
        uint32_t t_stack_size = array_count(context->transition_stack);
        for (uint32_t j = state_stack_index + 1; j < t_stack_size; i++) {
            transition_ref jth_item = array_get(context->transition_stack, j);

            if (threads_equal(proc(jth_item), trans->thread) && false) {
                thread_ref new_thread = thread_copy(thread);
                array_append(E, new_thread);
            }
        }
    }
    return E;
}

void
dynamically_update_backtrack_sets(dpor_context_ref context, state_stack_item_ref ref)
{
    if (!ref || !context) return;

    uint32_t thread_count = array_count(ref->state->threads);
    for (uint32_t i = 0; i < thread_count; i++) {
        thread_ref thread = array_get(ref->state->threads, i);
        transition_ref enabled = shared_state_get_first_enabled_transition_by_thread(ref->state, thread);
        if (enabled != NULL) {
            int i = latest_dependent_coenabled_transition_index(context, enabled);
            if (i < 0) continue;

            state_stack_item_ref from_state = array_get(context->state_stack, i);
            array_ref enabled_at_state = shared_state_create_enabled_transitions(ref->state);
            array_ref E = compute_set_E(context, enabled_at_state, thread, i);

            if (array_is_empty(E)) {
                array_append_array(from_state->backtrack_set, E);
            } else {
                array_append(from_state->backtrack_set, array_get_first(E));
            }

            array_destroy(E, NULL);
        }
    }
}