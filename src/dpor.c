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

/* Resides in shared memory -> data child passes back to parent */
/* The shared semaphores must also reside in shared memory as per the man page */
pid_t cpid = -1;
void *shm_addr = NULL;
shm_transition_ref shm_child_result = NULL;
mc_shared_cv (*queue)[MAX_TOTAL_THREADS_PER_SCHEDULE] = NULL;

const size_t allocation_size = sizeof(*shm_child_result) + sizeof(*queue);

// __attribute__((constructor))
void
dpor_init(void)
{
    dpor_initialize_shared_memory_region();
    csystem_init(&csystem);

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
    if (is_child) return;
    puts("***** Model checking completed! *****");
    exit(0);
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
    for (int i = 0; i < MAX_TOTAL_THREADS_PER_SCHEDULE; i++) {
        mc_shared_cv_destroy(&(*queue)[i]);
        mc_shared_cv_init(&(*queue)[i]);
    }
}

static bool
dpor_begin_target_program_at_main(void)
{
    bool is_child = dpor_spawn_child();
    if (is_child) {
        // NOTE: Technically, the child will be frozen
        // inside of dpor_init until it is scheduled. But
        // this is only a technicality: it doesn't actually
        // matter where the child spawns so long as it reaches
        // the actual source program
        tid_self = 0;
        dpor_initialize_shared_memory_region();
        thread_await_dpor_scheduler_for_thread_start_transition();
    }
    return is_child;
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
    kill(cpid, SIGSTOP);
    cpid = -1;
}

static void
dpor_run_iterative_deepening_with_steps(uint32_t steps, transition_refc initial_transition)
{
    uint32_t depth = 0;
    transition_refc t_next = initial_transition;
    while (++depth <= steps) {
        tid_t tid = t_next->thread->tid;

        dpor_run_thread_to_next_visible_operation(tid);
        csystem_simulate_running_thread(&csystem, shm_child_result, t_next->thread);
        csystem_dynamically_update_backtrack_sets(&csystem);

        t_next = csystem_get_first_enabled_transition(&csystem);
        if (t_next == NULL) {
            // TODO: We've hit a deadlock attempting to fill
            // the state stack with `max_depth` items
            // Perhaps we should retry entirely with a smaller
            // depth
            dpor_child_kill();
            puts("DEADLOCK DETECTED");
            abort();
        }
    }
    dpor_child_kill();
}

static bool
dpor_backtrack_main(uint32_t max_depth, transition_refc initial_transition)
{
    if (max_depth == 0) return false; // Nothing to do

    bool is_child = dpor_spawn_child_following_transition_stack();
    if (is_child) return true;

    dpor_run_iterative_deepening_with_steps(max_depth, initial_transition);
    return false;
}

/* Return true if we should run another iteration */
static bool
dpor_parent_scheduler_main(uint32_t max_depth)
{
    dpor_reset_cv_locks();
    csystem_reset(&csystem);
    dpor_register_main_thread();

    bool is_child = dpor_begin_target_program_at_main();
    if (is_child) return true;

    thread_ref main_thread = thread_get_self();
    transition_ref t_slot_for_main_thread_next = csystem_get_transition_slot_for_thread(&csystem, main_thread);
    dpor_init_thread_start_transition(t_slot_for_main_thread_next, main_thread);
    dpor_run_iterative_deepening_with_steps(max_depth, t_slot_for_main_thread_next);

    while (!csystem_state_stack_is_empty(&csystem)) {
        state_stack_item_ref s_top = csystem_state_stack_top(&csystem);

        // Calls to `dpor_backtrack_main` push
        // new items onto the state and transition
        // stacks; hence, "depth--;" is not sufficient
        const uint32_t depth = csystem_transition_stack_count(&csystem);
        mc_assert(depth <= max_depth);
        printf("**** Backtracking at depth %d ****\n", depth);
        while (!hash_set_is_empty(s_top->backtrack_set)) {
            // TODO: We can be smart here and only run a thread
            // if it is not already in a sleep set or lock set (eventually)
            transition_ref t_initial = csystem_pop_first_enabled_transition_in_backtrack_set(&csystem, s_top);
            is_child = dpor_backtrack_main(max_depth - depth, t_initial);
            if (is_child) return true;
        }
        printf("**** Backtracking completed at depth %d ****\n", depth);
        hash_set_destroy(s_top->done_set);
        hash_set_destroy(s_top->backtrack_set);
        csystem_pop_program_stacks_for_backtracking(&csystem);
    }

    return false;
}

static bool
dpor_parent_scheduler_loop(uint32_t max_depth)
{
    for (uint32_t i = 1; i < max_depth; i++) {
        printf("Scheduler main invoked for %d steps\n", i);
        if (dpor_parent_scheduler_main(i)) // True for the child so it can escape to the main routine
            return true;
    }
    return false;
}

// ****** CHILD FUNCTIONS **** \\

static bool
dpor_spawn_child_following_transition_stack(void)
{
    dpor_reset_cv_locks();
    bool is_child = dpor_begin_target_program_at_main();

    if (!is_child) {
        int transition_stack_height = csystem_transition_stack_count(&csystem);
        for (int i = 0; i < transition_stack_height; i++) {
            // NOTE: This is reliant on the fact
            // that threads are created in the same order
            // when we create them. This will always be consistent,
            // but we might need to look out for when a thread dies
            transition_ref t = csystem_transition_stack_get_element(&csystem, i);
            dpor_run_thread_to_next_visible_operation(t->thread->tid);
        }
    } else {
        // We need to reset the concurrent system
        // for the child since, at the time this method
        // is invoked, it will have a complete copy of
        // the state the of system. But we need to
        // re-simulate the system by running the transitions
        // in the transition stack; otherwise, shadow resource
        // allocations will be off
        csystem_reset(&csystem);
        dpor_register_main_thread();
    }

    return is_child;
}

static void
dpor_run_thread_to_next_visible_operation(tid_t tid)
{
    mc_assert(tid != TID_INVALID);
    mc_shared_cv_ref cv = &(*queue)[tid];
    mc_shared_cv_wake_thread(cv);
    mc_shared_cv_wait_for_thread(cv);
}

// NOTE: Assumes that the parent process
// is asleep (called dpor_run_thread_to_next_visible_operation); the behavior
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
thread_await_dpor_scheduler_for_thread_start_transition(void)
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
