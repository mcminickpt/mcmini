#include "dpor.h"
#include "mc_shared_cv.h"
#include "fail.h"
#include "shm.h"
#include <assert.h>
#include <fcntl.h>
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
    dpor_register_main_thread();

    bool is_child = dpor_spawn_child();
    if (is_child) return true;

    
    thread_ref main_thread = thread_get_self();
    transition_ref t_slot_for_main_thread_next = csystem_get_transition_slot_for_thread(&csystem, main_thread);
    dpor_init_thread_start_transition(t_slot_for_main_thread_next, main_thread);

    state_stack_item_ref s_top = csystem_grow_state_stack(&csystem);
    transition_ref t_top = csystem_grow_transition_stack(&csystem, main_thread);

    uint32_t depth = 0;
    while (++depth <= max_depth) {
        tid_t tid = t_top->thread->tid;
        dpor_run(tid);

        // Update the transition that this thread does next based
        // on the result the child wrote to shared memory
        transition_ref tslot = csystem_get_transition_slot_for_thread(&csystem, t_top->thread);
        copy_into_transition_from_shm(shm_child_result, tslot);
        dynamically_update_backtrack_sets();

        s_top = csystem_grow_state_stack(&csystem);
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
    while (!csystem_state_stack_is_empty(&csystem)) {
        s_top = csystem_shrink_state_stack(&csystem);
        t_top = csystem_shrink_transition_stack(&csystem);

        // Calls to `dpor_backtrack_main` push
        // new items onto the state and transition
        // stacks; hence, "depth--;" is not sufficient
        depth = csystem_state_stack_count(&csystem);

        mc_assert(depth <= max_depth);
        if (!hash_set_is_empty(s_top->backtrack_set)) {
            bool is_child = dpor_backtrack_main(s_top, max_depth - depth);
            if (is_child) { return true; }
        }

        // TODO: Remove backtrack and done sets to prevent memory leaks
    }

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
dynamically_update_backtrack_sets(void)
{
    csystem_start_backtrack(&csystem);

    transition enabled_transitions_s_top[MAX_TOTAL_THREADS_PER_SCHEDULE];
    transition scratch_enabled_at_i[MAX_TOTAL_THREADS_PER_SCHEDULE];

    // The set of threads we have already found the latest dependent,
    // co-enabled transition for
    hash_set_ref threads_processed = hash_set_create((hash_function)thread_hash);

    // Maps `thread_ref` to a `transition_ref` pointing
    // into the static array `transitions_s_top`
    hash_table_ref s_top_enabled_transition_map = hash_table_create();
    hash_table_set_hash_function(s_top_enabled_transition_map, (hash_function) thread_hash);

    int thread_count = csystem_get_thread_count(&csystem);
    int t_stack_height = csystem_transition_stack_count(&csystem);

    // 1. Compute enabled transition map
    {
        int transitions_s_top_index = 0;
        for (tid_t tid = 0; tid < thread_count; tid++) {
            thread_ref tid_thread = csystem_get_thread_with_tid(&csystem, tid);
            transition_ref tid_transition = csystem_get_transition_slot_for_thread(&csystem, tid_thread);

            if (transition_enabled(tid_transition)) {
                transition_ref static_slot = &enabled_transitions_s_top[transitions_s_top_index++];
                *static_slot = *tid_transition;

                hash_table_set_implicit(s_top_enabled_transition_map,
                                        tid_thread,
                                        static_slot);
            }
        }
    }

    for (int i = t_stack_height - 1; i >= 0; i--) {

        if (hash_table_is_empty(s_top_enabled_transition_map))
            break; /* Don't need to keep going further into the transition stack if everything is processed */

        int num_enabled_at_s_i = csystem_copy_enabled_transitions(&csystem, scratch_enabled_at_i);
        transition_ref t_top_i = csystem_transition_stack_top(&csystem);
        state_stack_item_ref s_top_i = csystem_state_stack_top(&csystem);

        for (int tid = 0; tid < thread_count; tid++) {
            thread_ref s_top_thread = csystem_get_thread_with_tid(&csystem, tid);
            transition_ref enabled_transition_maybe = NULL; // At s_top original
            if ((enabled_transition_maybe = hash_table_get_implicit(s_top_enabled_transition_map, s_top_thread)) != NULL) {

                bool found_max_i_condition = transitions_coenabled(t_top_i, enabled_transition_maybe) &&
                                             transitions_dependent(t_top_i, enabled_transition_maybe) &&
                                             !happens_before_thread(&csystem, i, s_top_thread);
                if (found_max_i_condition) {

                    transition_ref p_transition = enabled_transition_maybe;
                    thread_ref p = p_transition->thread;

                    for (int s_i_index = 0; s_i_index < num_enabled_at_s_i; s_i_index++) {
                        transition_ref q_transition = &scratch_enabled_at_i[s_i_index];
                        thread_ref q = q_transition->thread;

                        bool q_is_in_E = threads_equal(q, p) && csystem_p_q_could_race(&csystem, i, q, p);
                        if (q_is_in_E) {
                            hash_set_insert(s_top_i->backtrack_set, q);

                            // TODO: Only one q is needed. An optimization
                            // where we check the done set for q first would
                            // reduce backtracking

                            // We're done now
                            goto search_over;
                        }
                    }

                    // If we exit the loop without hitting the `goto`, we
                    // know that `E` is the empty set. In this case, we must
                    // add every enabled thread to the backtrack set
                    for (int s_i_index = 0; s_i_index < num_enabled_at_s_i; s_i_index++) {
                        transition_ref q_transition = &scratch_enabled_at_i[s_i_index];
                        thread_ref q = q_transition->thread;
                        hash_set_insert(s_top_i->backtrack_set, q);
                    }

                    search_over:
                    // Remove this thread from needing to be searched again
                    hash_table_remove_implicit(s_top_enabled_transition_map, s_top_thread);
                }
            }
        }
        csystem_shrink_transition_stack(&csystem);
    }
    csystem_end_backtrack(&csystem);
}

static void
dpor_init_thread_transition(transition_ref tref, thread_ref thread, thread_operation_type type)
{
    thread_operation top;
    top.type = type;
    top.thread = thread;

    tref->operation.type = THREAD_LIFECYCLE;
    tref->operation.thread_operation = top;
    tref->thread = thread;
}

inline void
dpor_init_thread_start_transition(transition_ref transition, thread_ref thread)
{
    dpor_init_thread_transition(transition, thread, THREAD_START);
}

inline void
dpor_init_thread_finish_transition(transition_ref transition, thread_ref thread)
{
    dpor_init_thread_transition(transition, thread, THREAD_FINISH);
}