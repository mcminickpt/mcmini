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

/* Resides in shared memory -> data child passes back to parent */
/* The shared semaphores must also reside in shared memory as per the man page */
pid_t cpid = -1;
void *shm_addr = NULL;
shm_transition_ref shm_child_result = NULL;
mc_shared_cv (*queue)[MAX_TOTAL_THREADS_PER_SCHEDULE] = NULL;
const size_t allocation_size = sizeof(*shm_child_result) + sizeof(*queue);

void dpor_sigusr2(int sig)
{
    dpor_child_exit();
}

// __attribute__((constructor))
void
dpor_init(void)
{
    csystem_init(&csystem);
    dpor_initialize_shared_memory_region();

    // XPC between parent (scheduler) and child (the program)
    atexit(&dpor_child_kill);

    // We need to create the semaphores that are shared
    // across processes BEFORE the threads are created; otherwise
    // when we find new threads in the child process, we won't have
    // a way to communicate between the parent process and child
    // if we don't already have a semaphore in shared memory
    for (int i = 0; i < MAX_TOTAL_THREADS_PER_SCHEDULE; i++)
        mc_shared_cv_init(&(*queue)[i]);
    dpor_register_main_thread();
    bool is_child = dpor_parent_scheduler_loop(MAX_VISIBLE_OPERATION_DEPTH);
    if (is_child) {
        // NOTE: Since this is a fork of the parent, we need
        // only map the shared memory and don't have
        // to re-point `shm_child_result` and `queue` to the appropriate
        // locations (since this was already done above)
        dpor_initialize_shared_memory_region();
        thread_await_dpor_scheduler_initialization();
    } else { // Parent
        puts("Model checking completed");
        exit(0);
    }
}

static void
dpor_initialize_shared_memory_region()
{
    shm_addr = dpor_create_shared_memory_region();
    shm_child_result = shm_addr;
    queue = shm_addr + sizeof(*shm_child_result);
}

static void*
dpor_create_shared_memory_region(void)
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
    fsync(fd);
    close(fd);
    return shm_addr;
}

static void
dpor_reset_cv_locks(void)
{
    // TODO: Technically, the semaphores don't need to be destroyed,
    // just set to the appropriate starting values
    for (int i = 0; i < MAX_TOTAL_THREADS_PER_SCHEDULE; i++) {
        mc_shared_cv_destroy(&(*queue)[i]);
        mc_shared_cv_init(&(*queue)[i]);
    }
}

static bool /* */
dpor_spawn_child(void)
{
    // Ensure that a child does not already exist
    // to prevent fork bombing
    mc_assert(cpid == -1);

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
    int r = kill(cpid, SIGSTOP);
    cpid = -1;
}

static bool
dpor_backtrack_main(state_stack_item_ref ss_item, uint32_t max_depth)
{
    bool is_child = dpor_spawn_child_following_transition_stack();
    if (is_child) return true;

    uint32_t depth = 0;
    state_stack_item_ref s_top = ss_item;
    transition_ref t_top = state_stack_item_get_first_enabled_backtrack_transition(ss_item);
    if (t_top == NULL) {
        // TODO: We've hit a deadlock. There are no enabled
        // transitions at this point and we should print that
        printf("DEADLOCK\n");
        abort();
    }
    state_stack_item_remove_backtrack_thread(ss_item, t_top->thread);
    state_stack_item_mark_thread_as_searched(ss_item, t_top->thread);

//    do {
//        array_append(s_stack, s_top);
//        array_append(t_stack, t_top);
//        dpor_run(t_top->thread->tid);
//
//        transition_ref new_transition = create_transition_from_shm(shm_child_result);
//        {
//            state_stack_item_ref new_s_top = state_stack_item_alloc();
//            shared_state_ref new_s = next(s_top->state, t_top, new_transition);
//            new_s_top->state = new_s;
//            new_s_top->backtrack_set = array_create();
//            new_s_top->done_set = array_create();
//            s_top = new_s_top;
//        }
//
//        dynamically_update_backtrack_sets(s_top);
//        t_top = shared_state_get_first_enabled_transition(s_top->state);
//    } while (depth++ < max_depth);

    return false;
}

/* Return true if we should run another iteration */
static bool
dpor_parent_scheduler_main(uint32_t max_depth)
{
    dpor_reset_cv_locks();
    csystem_reset(&csystem);

    bool is_child = dpor_spawn_child();
    if (is_child) return true;

    state_stack_item_ref s_top = csystem_grow_state_stack(&csystem);

    thread_ref main_thread = thread_get_self();
    dpor_create_thread_start_transition(main_thread);

    transition_ref t_top = csystem_grow_transition_stack(&csystem, main_thread);

    uint32_t depth = 0;
    while (++depth <= max_depth) {
        tid_t tid = t_top->thread->tid;
        dpor_run(tid);

        // Update the transition that this thread does next based
        // on the result the child wrote to shared memory
        transition_ref tslot = csystem_get_transition_slot_for_thread(&csystem, t_top->thread);
        copy_into_transition_from_shm(shm_child_result, tslot);

        state_stack_item_ref next_s_top = csystem_grow_state_stack(&csystem);
        s_top = next_s_top;
        dynamically_update_backtrack_sets(s_top);
        t_top = csystem_get_first_enabled_transition(&csystem);

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
//    while (!array_is_empty(s_stack)) {
//        s_top = array_remove_last(s_stack);
//        t_top = array_remove_last(t_stack);
//
//        // Calls to `dpor_backtrack_main` push
//        // new items onto the state and transition
//        // stacks; hence, "depth--;" is not sufficient
//        depth = array_count(s_stack);
//
//        mc_assert(depth <= max_depth);
//        if (!array_is_empty(s_top->backtrack_set)) {
//            bool is_child = dpor_backtrack_main(s_top, max_depth - depth);
//            if (is_child) { return true; }
//        }
//        state_stack_item_destroy(s_top);
//        transition_destroy(t_top);
//    }

    return false;
}

static bool
dpor_parent_scheduler_loop(uint32_t max_depth)
{
    for (uint32_t i = 1; i < max_depth; i++)
        if (dpor_parent_scheduler_main(i)) // True for the child so it can escape to the main routine
            return true;
    return false;
}

// ****** CHILD FUNCTIONS **** \\

static bool
dpor_spawn_child_following_transition_stack(void)
{
    bool is_child = dpor_spawn_child();

//    if (!is_child) {
//        uint32_t count = array_count(t_stack);
//        for (uint32_t i = 0; i < count; i++) {
//            // NOTE: This is reliant on the fact
//            // that threads are created in the same order
//            // when we create them. This will always be consistent,
//            // but we might need to look out for when a thread dies
//            transition_ref t = array_get(t_stack, i);
//            dpor_run(t->thread->tid);
//        }
//    }

    return is_child;
}

static void
dpor_run(tid_t tid)
{
    mc_assert(tid != TID_INVALID);
    mc_shared_cv_ref cv = &(*queue)[tid];
    mc_shared_cv_wake_thread(cv);
    mc_shared_cv_wait_for_thread(cv);
}

// NOTE: Assumes that the parent process
// is asleep (called dpor_run); the behavior
// is undefined otherwise
void
thread_await_dpor_scheduler(void)
{
    mc_assert(tid_self != TID_INVALID);
    mc_shared_cv_ref cv = &(*queue)[tid_self];
    mc_shared_cv_wake_scheduler(cv);
    mc_shared_cv_wait_for_scheduler(cv);
}

// NOTE: This should only be called in one location:
// When the scheduler starts, there is an initial
// race condition between the child process and the
// parent process with `thread_await_dpor_scheduler`. `thread_await_dpor_scheduler` assumes
// the the scheduler (parent) process is asleep; but upon
// initialization this is not true. Hence, this method is
// invoked instead
void
thread_await_dpor_scheduler_initialization(void)
{
    mc_assert(tid_self != TID_INVALID);
    mc_shared_cv_ref cv = &(*queue)[tid_self];
    mc_shared_cv_wait_for_scheduler(cv);
}

static tid_t
dpor_register_thread(void)
{
    return csystem_register_thread(&csystem);
}

static tid_t
dpor_register_main_thread(void)
{
    return csystem_register_main_thread(&csystem);
}

static void
dpor_child_exit(void)
{
    printf("CHILD EXITED %d\n", (int)getpid());
    exit(0);
}

// ****** CHILD FUNCTIONS END **** //

static int
latest_dependent_coenabled_transition_index(transition_ref transition)
{
//    if (!transition) return -1;
//    if (array_is_empty(t_stack)) return -1;
//
//    uint32_t transition_stack_size = array_count(t_stack);
//
//    // NOTE: The second condition for the loop is due to unsigned integer
//    // wrapped (defined behavior)
//    for (uint32_t i = transition_stack_size - 1; i >= 0 && i < transition_stack_size; i--) {
//        transition_ref transition_i = array_get(t_stack, i);
//        if (transitions_dependent(transition_i, transition) && transitions_coenabled(transition_i, transition)) {
//            return (int)i;
//        }
//    }
//    return -1;
}

static array_ref
compute_set_E(transition_array_ref enabled_transitions,
              thread_ref thread,
              int state_stack_index)
{
//    if (!enabled_transitions || !thread) return NULL;
//    thread_array_ref E = array_create();
//    if (!E) return NULL;
//
//    uint32_t nts = array_count(enabled_transitions);
//    for (uint32_t i = 0; i < nts; i++) {
//        transition_ref enabled_t = array_get(enabled_transitions, i);
//
//        if (threads_equal(thread, enabled_t->thread)) {
//            thread_ref new_thread = thread_copy(thread);
//            array_append(E, new_thread);
//        }
//
//        uint32_t t_stack_size = array_count(t_stack);
//        for (uint32_t j = state_stack_index + 1; j < t_stack_size; j++) {
//            transition_ref jth_item = array_get(t_stack, j);
//
//            if (threads_equal(proc(jth_item), enabled_t->thread)) {
//                thread_ref new_thread = thread_copy(thread);
//                array_append(E, new_thread);
//            }
//        }
//    }
//    return E;
}

void
dynamically_update_backtrack_sets(state_stack_item_ref ref)
{
//    if (!ref) return;
//
//    uint32_t thread_count = array_count(ref->state->threads);
//    for (uint32_t j = 0; j < thread_count; j++) {
//        thread_ref thread = array_get(ref->state->threads, j);
//        transition_ref enabled = shared_state_get_first_enabled_transition_by_thread(ref->state, thread);
//
//        if (enabled != NULL) {
//            int i = latest_dependent_coenabled_transition_index(enabled);
//            if (i < 0) continue;
//
//            state_stack_item_ref from_state = array_get(s_stack, i);
//            array_ref enabled_at_state = shared_state_create_enabled_transitions(ref->state);
//            array_ref E = compute_set_E(enabled_at_state, thread, i);
//
//            if (array_is_empty(E)) {
//                array_append_array(from_state->backtrack_set, E);
//            } else {
//                array_append(from_state->backtrack_set, array_get_first(E));
//            }
//            array_destroy(E, NULL);
//        }
//    }
}

static transition_ref
dpor_create_thread_transition(thread_ref thread, thread_operation_type type)
{
    thread_operation top;
    top.type = type;
    top.thread = thread;

    transition_ref trans = csystem_get_transition_slot_for_thread(&csystem, thread);
    trans->operation.type = THREAD_LIFECYCLE;
    trans->operation.thread_operation = top;
    trans->thread = thread;

    return trans;
}

transition_ref
dpor_create_thread_start_transition(thread_ref thread)
{
    return dpor_create_thread_transition(thread, THREAD_START);
}

transition_ref
dpor_create_thread_finish_transition(thread_ref thread)
{
    return dpor_create_thread_transition(thread, THREAD_FINISH);
}