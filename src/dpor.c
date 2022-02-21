#include "dpor.h"
#include "cooplock.h"
#include "shm.h"
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PTHREAD_SUCCESS (0)
thread_local tid_t thread_self = TID_INVALID;

state_stack_ref s_stack = NULL;
transition_array_ref t_stack = NULL;

tid_t tid_next = tid_main_thread;
thread threads[MAX_TOTAL_THREADS_PER_SCHEDULE]; /* MAX_TOTAL_THREADS_PER_SCHEDULE size */
coop_lock queue[MAX_TOTAL_THREADS_PER_SCHEDULE];

/* Resides in shared memory -> data child passes back to parent */
shm_transition_ref shm_child_result;

thread_ref
thread_get_self(void)
{
    if (thread_self == TID_INVALID) return NULL;
    return &threads[thread_self];
}

void dpor_run(tid_t tid);
void dpor_parent_scheduler_loop(uint32_t max_depth);


// __attribute__((constructor))
void
dpor_init(void)
{
    dpor_parent_scheduler_loop(MAX_VISIBLE_OPERATION_DEPTH);
}

static bool /* */
dpor_spawn_child(void)
{
    pid_t childpid;
    if ( (childpid = fork()) < 0) {
        perror("fork");
        abort();
    }

    if (FORK_IS_CHILD_PID(childpid)) {
        return true; /* true signifies the child */
    }
    return false;
}

/* Return true if we should run another iteration */
static bool
dpor_parent_scheduler_main(uint32_t max_depth)
{
    s_stack = array_create();
    t_stack = array_create();
    dpor_register_main_thread();

    thread_ref main_thread = thread_get_self();
    state_stack_item_ref initial_stack_item = state_stack_item_create();

    transition_ref main_thread_transition = create_retain_thread_start_transition(main_thread);
    array_append(initial_stack_item->state->transitions, main_thread_transition);

    uint32_t depth = 0;
    state_stack_item_ref s = initial_stack_item;
    transition_ref t = main_thread_transition;
    while (depth++ <= max_depth && t != NULL) {
        // Push s and t onto the stack
        array_append(dpor_shared->state_stack, s);
        array_append(dpor_shared->transition_stack, t);

        bool is_child = dpor_spawn_child();
        if (is_child) return false; // Child process exiting the scheduler

        // TODO: Extract next `t` from having run the child process
        tid_t tid = t->thread->tid;
        dpor_run(tid);

        // TODO: Implement this function
        transition newest_transition = convert_to_transition(shm_child_result.shm_transition);

        // TODO: Update the state with the newest_transition
        // s = next(s, t);

        dynamically_update_backtrack_sets(dpor_shared, s);
        t = shared_state_get_first_enabled_transition(s->state);
    }

    // TODO: Do actual backtracking


    return true;
}

static void
dpor_parent_scheduler_loop(uint32_t max_depth)
{
    for (uint32_t i = 0; i < max_depth; i++)
        if (!dpor_parent_scheduler_main(max_depth)) // False for the child so it can escape to the main program
            return;
}

// ****** CHILD FUNCTIONS **** \\

tid_t
dpor_register_thread(void)
{
    tid_t self = tid_next++;
    thread_self = self;
    thread_ref tself = &threads[self];
    tself->owner = pthread_self();
    tself->tid = self;
    tself->is_alive = true;
    return self;
}

inline tid_t
dpor_register_main_thread(void)
{
    tid_t main = dpor_register_thread();
    assert(main == TID_MAIN_THREAD);
    return main;
}

static void
dpor_run_thread(tid_t tid)
{
    coop_lock_ref lock = &queue[tid];
    coop_wake_thread(lock);
    coop_wait_thread(lock);
}

void
thread_await_dpor_scheduler(void)
{
    coop_lock_ref lock = &queue[thread_self];
    coop_wake_scheduler(lock);
    coop_wait_scheduler(lock);
}

static void
dpor_child_exit(void)
{
    exit(0);
}

// ****** CHILD FUNCTIONS END **** //


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
compute_set_E(transition_array_ref enabled_transitions,
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

        uint32_t t_stack_size = array_count(context->transition_stack);
        for (uint32_t j = state_stack_index + 1; j < t_stack_size; i++) {
            transition_ref jth_item = array_get(context->transition_stack, j);

            if (threads_equal(proc(jth_item), trans->thread)) {
                thread_ref new_thread = thread_copy(thread);
                array_append(E, new_thread);
            }
        }
    }
    return E;
}


void
dynamically_update_backtrack_sets(state_stack_item_ref ref)
{
    if (!ref) return;

    uint32_t thread_count = array_count(ref->state->threads);
    for (uint32_t i = 0; i < thread_count; i++) {
        thread_ref thread = array_get(ref->state->threads, i);
        transition_ref enabled = shared_state_get_first_enabled_transition_by_thread(ref->state, thread);

        if (enabled != NULL) {
            int i = latest_dependent_coenabled_transition_index(context, enabled);
            if (i < 0) continue;

            state_stack_item_ref from_state = array_get(s_stack, i);
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