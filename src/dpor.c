#include "dpor.h"
#include "mc_shared_cv.h"
#include "fail.h"
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
pid_t cpid = -1;

state_stack_ref s_stack = NULL;
transition_array_ref t_stack = NULL;

tid_t tid_next = TID_MAIN_THREAD;
thread threads[MAX_TOTAL_THREADS_PER_SCHEDULE]; /* MAX_TOTAL_THREADS_PER_SCHEDULE size */


/* Resides in shared memory -> data child passes back to parent */

void *shm_addr = NULL;
shm_transition_ref shm_child_result = NULL;
mc_shared_cv (*queue)[MAX_TOTAL_THREADS_PER_SCHEDULE] = NULL;

const size_t allocation_size = sizeof(*shm_child_result) + MAX_TOTAL_THREADS_PER_SCHEDULE * sizeof(*queue);

void dpor_sigusr1(int sig)
{
    dpor_child_exit();
}

// __attribute__((constructor))
void
dpor_init(void)
{
    s_stack = array_create();
    t_stack = array_create();
    shm_addr = dpor_init_shared_memory_region();

    shm_child_result = shm_addr;
    queue = shm_addr + sizeof(*shm_child_result);

    // We need to create the semaphores that are shared
    // across processes BEFORE the threads are created; otherwise
    // when we find new threads in the child process, we won't have
    // a way to communicate between the parent process and child
    // if we don't already have a shared memory semaphore
    for (int i = 0; i < MAX_TOTAL_THREADS_PER_SCHEDULE; i++)
        mc_shared_cv_init(&*queue[i]);
    dpor_register_main_thread();
    atexit(&dpor_child_kill);
    bool is_child = dpor_parent_scheduler_loop(MAX_VISIBLE_OPERATION_DEPTH);

    if (is_child) {
        shm_child_result = dpor_init_shared_memory_region();
        printf("THREAD AWAIT\n");
        thread_pretty(thread_get_self());
        thread_await_dpor_scheduler();
        printf("THREAD AWAIT 2\n");
    } else { // Parent
        exit(0);
    }
}

static void*
dpor_init_shared_memory_region(void)
{
    //  If the region exists, then this returns a fd for the existing region.
    //  Otherwise, it creates a new shared memory region.
    char dpor[100];
    snprintf(dpor, sizeof(dpor), "/DPOR-%s", getenv("USER"));
    // This creates a file in /dev/shm/
    int fd = shm_open(dpor, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        if (errno == EACCES) {
            fprintf(stderr, "Shared memory region '%s' not owned by this process\n",
                    dpor);
        } else {
            perror("shm_open");
        }
        exit(EXIT_FAILURE);
    }
    int rc = ftruncate(fd, allocation_size);
    if (rc == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    // We want stack at same address for each process.  Otherwise, a pointer
    //   to an address in the stack data structure will not work everywhere.
    //   Hopefully, this address is not already used.
    void *stack_address = (void *)0x4444000;
    shm_transition_ref *shm_addr = mmap(stack_address, allocation_size,
                       PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0);
    if (shm_addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    // shm_unlink(dpor); // Don't unlink while child processes need to open this.
    close(fd);
    return shm_addr;
}

static bool /* */
dpor_spawn_child(void)
{
    // TODO: Ensure that a child does not already exist
    pid_t childpid;
    if ( (childpid = fork()) < 0) {
        perror("fork");
        abort();
    }
    cpid = childpid;

    if (FORK_IS_CHILD_PID(childpid)) {
        return true; /* true signifies the child */
    }
    return false;
}

static void
dpor_child_kill(void)
{
    if (cpid == -1) return; // No child
    int r = kill(cpid, SIGUSR1);
    cpid = -1;
}

static bool
dpor_backtrack_main(state_stack_item_ref ss_item, uint32_t max_depth)
{
    bool is_child = dpor_spawn_child_following_transition_stack();
    if (is_child) return true;

    uint32_t depth = 0;
    state_stack_item_ref s_top = ss_item;

    // TODO: Pick a transition from the backtrack set
    transition_ref t_top; // t = t in ss_item->state enabled transitions & t.tid in backtrack set

    // TODO: Remove t.tid from the backtrack set

    // TODO: Add t.tid to the done set

    do {
        array_append(s_stack, s_top);
        array_append(t_stack, t_top);
        dpor_run(t_top->thread->tid);

        transition_ref new_transition = create_transition_from_shm(shm_child_result);

        // TODO: s_top = next(s_top, t_top) + add new_transition to to the s_top->state->transitions
        // Roughly
        // s_top = next(s_top, t_top)
        // s_top->state->transitions <--- add new_transition here and get rid of old one by this thread

        dynamically_update_backtrack_sets(s_top);

        // TODO: Choose a new enabled transition to run next
        // t =

    } while (depth++ < max_depth);

    return false;
}

/* Return true if we should run another iteration */
static bool
dpor_parent_scheduler_main(uint32_t max_depth)
{
    bool is_child = dpor_spawn_child();
    if (is_child) return true;

    thread_ref main_thread = thread_get_self();
    state_stack_item_ref initial_stack_item = state_stack_item_create();

    transition_ref main_thread_transition = create_retain_thread_start_transition(main_thread);
    array_append(initial_stack_item->state->transitions, main_thread_transition);

    uint32_t depth = 0;
    state_stack_item_ref s_top = initial_stack_item;
    transition_ref t_top = main_thread_transition;

    while (depth++ <= max_depth) {
        array_append(s_stack, s_top);
        array_append(t_stack, t_top);

        tid_t tid = t_top->thread->tid;
        dpor_run(tid);

        transition_ref newest_transition = create_transition_from_shm(shm_child_result);
        state_stack_item_ref next_s_top = state_stack_item_alloc();
        next_s_top->backtrack_set = array_create();
        next_s_top->done_set = array_create();
        next_s_top->state = next(s_top->state, t_top, newest_transition);
        s_top = next_s_top;

        dynamically_update_backtrack_sets(s_top);
        t_top = shared_state_get_first_enabled_transition(s_top->state);

        if (t_top == NULL) {
            // TODO: We've hit a deadlock attempting to fill
            // the state stack with `max_depth` items
            // Perhaps we should retry entirely with a smaller
            // depth
            dpor_child_kill();
            abort();
        }
    }

    dpor_child_kill();
    while (!array_is_empty(s_stack)) {
        s_top = array_remove_last(s_stack);
        t_top = array_remove_last(t_stack);

        // Calls to `dpor_backtrack_main` push
        // new items onto the state and transition
        // stacks; hence, "depth--;" is not sufficient
        depth = array_count(s_stack);

        mc_assert(depth <= max_depth);
        if (!array_is_empty(s_top->backtrack_set)) {
            bool is_child = dpor_backtrack_main(s_top, max_depth - depth);
            if (is_child) { return true; }
        }

        state_stack_item_destroy(s_top);
        transition_destroy(t_top);
    }

    return false;
}

static bool
dpor_parent_scheduler_loop(uint32_t max_depth)
{
    for (uint32_t i = 0; i < max_depth; i++)
        if (dpor_parent_scheduler_main(max_depth)) // True for the child so it can escape to the main routine
            return true;
    return false;
}

// ****** CHILD FUNCTIONS **** \\

static bool
dpor_spawn_child_following_transition_stack(void)
{
    bool is_child = dpor_spawn_child();

    if (!is_child) {
        uint32_t count = array_count(t_stack);
        for (uint32_t i = 0; i < count; i++) {
            transition_ref t = array_get(t_stack, i);
            dpor_run(t->thread->tid);
        }
    }

    return is_child;
}

tid_t
dpor_register_thread(void)
{
    tid_t self = tid_next++;
    thread_self = self;
    thread_ref tself = &threads[self];
    tself->owner = pthread_self();
    tself->tid = self;
    tself->is_alive = true;
    tself->global = true;
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
dpor_run(tid_t tid)
{
    mc_shared_cv_ref cv = &queue[tid];
    mc_shared_cv_wake_thread(cv);
    mc_shared_cv_wait_for_thread(cv);
}

void
thread_await_dpor_scheduler(void)
{
    mc_shared_cv_ref cv = &queue[thread_self];
    mc_shared_cv_wake_scheduler(cv);
    mc_shared_cv_wait_for_scheduler(cv);
}

static void
dpor_child_exit(void)
{
    exit(0);
}

// ****** CHILD FUNCTIONS END **** //

static int
latest_dependent_coenabled_transition_index(transition_ref transition)
{
    if (!transition) return -1;
    if (array_is_empty(t_stack)) return -1;

    uint32_t transition_stack_size = array_count(t_stack);
    for (uint32_t i = transition_stack_size - 1; i >= 0; i--) {
        transition_ref transition_i = array_get(t_stack, i);
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
        transition_ref enabled_t = array_get(enabled_transitions, i);

        if (threads_equal(thread, enabled_t->thread)) {
            thread_ref new_thread = thread_copy(thread);
            array_append(E, new_thread);
        }

        uint32_t t_stack_size = array_count(t_stack);
        for (uint32_t j = state_stack_index + 1; j < t_stack_size; j++) {
            transition_ref jth_item = array_get(t_stack, j);

            if (threads_equal(proc(jth_item), enabled_t->thread)) {
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
    for (uint32_t j = 0; j < thread_count; j++) {
        thread_ref thread = array_get(ref->state->threads, j);
        transition_ref enabled = shared_state_get_first_enabled_transition_by_thread(ref->state, thread);

        if (enabled != NULL) {
            int i = latest_dependent_coenabled_transition_index(enabled);
            if (i < 0) continue;

            state_stack_item_ref from_state = array_get(s_stack, i);
            array_ref enabled_at_state = shared_state_create_enabled_transitions(ref->state);
            array_ref E = compute_set_E(enabled_at_state, thread, i);

            if (array_is_empty(E)) {
                array_append_array(from_state->backtrack_set, E);
            } else {
                array_append(from_state->backtrack_set, array_get_first(E));
            }
            array_destroy(E, NULL);
        }
    }
}