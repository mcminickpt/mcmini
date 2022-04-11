#include "GMAL.h"
#include "GMAL_Private.h"
#include "GMALSharedTransition.h"
#include "GMALTransitionFactory.h"

extern "C" {
    #include "fail.h"
    #include "mc_shared_cv.h"
    #include <cassert>
    #include <fcntl.h>
    #include <cstdio>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
}

/* The shmTransitionData resides in shared memory */
/* The semaphores must also reside in shared memory as per the man page */

/* Synchronization primitives */
GMAL_THREAD_LOCAL tid_t tid_self = TID_INVALID;
pid_t cpid = -1;
mc_shared_cv (*threadQueue)[MAX_TOTAL_THREADS_IN_PROGRAM] = nullptr;
sem_t gmal_pthread_create_binary_sem;

/* Data transfer */
void *shmStart = nullptr;
GMALSharedTransition *shmTransitionData = nullptr;
const size_t shmAllocationSize =  sizeof(*threadQueue) + (sizeof(*shmTransitionData) + MAX_SHARED_MEMORY_ALLOCATION);

/* Program state */
GMALState programState;

// __attribute__((constructor))
void
gmal_init()
{
    gmal_create_program_state();
    gmal_initialize_shared_memory_region();
    gmal_create_thread_sleep_points();

    GMAL_FATAL_ON_FAIL(sem_init(&gmal_pthread_create_binary_sem, 0, 0) == 0);
    GMAL_FATAL_ON_FAIL(atexit(&gmal_exit) == 0);

    GMAL_PROGRAM_TYPE program = gmal_scheduler_main();
    if (GMAL_IS_SOURCE_PROGRAM(program)) return;

    puts("***** Model checking completed! *****");
    exit(EXIT_SUCCESS);
}

void
gmal_create_program_state()
{
    programState = GMALState();
}

GMAL_PROGRAM_TYPE
gmal_scheduler_main()
{
    gmal_register_main_thread();

    auto mainThread = programState.getThreadWithId(tid_self);
    auto mainThreadPtr = (GMALThread*)mainThread.get();
    auto initialTransition = GMALTransitionFactory::createInitialTransitionForThread(mainThreadPtr);
    programState.setNextTransitionForThread(tid_self, initialTransition.get());

//    programStat

//    thread_ref main_thread = thread_get_self();
//    dynamic_transition_ref t_slot_for_main_thread_next = csystem_get_transition_slot_for_thread(&csystem, main_thread);
//    dpor_init_thread_start_transition(t_slot_for_main_thread_next, main_thread);

    GMAL_PROGRAM_TYPE program = gmal_begin_target_program_at_main();
    if (GMAL_IS_SOURCE_PROGRAM(program))
        return GMAL_SOURCE_PROGRAM;

//    gmal_exhaust_threads(t_slot_for_main_thread_next);
//
//    while (!csystem_state_stack_is_empty(&csystem)) {
//        const uint32_t depth = csystem_state_stack_count(&csystem);
//        printf("**** Backtracking at depth %d ****\n", depth);
//
//        state_stack_item_ref s_top = csystem_state_stack_top(&csystem);
//        if (!hash_set_is_empty(s_top->backtrack_set)) {
//            // TODO: We can be smart here and only run a thread
//            // if it is not already in a sleep set or lock set (eventually)
//            dynamic_transition_ref t_initial = csystem_pop_first_enabled_transition_in_backtrack_set(&csystem, s_top);
//
//            if (t_initial) {
//                printf("**** Backtracking BEGUN at depth %d ****\n", depth);
//                program = gmal_readvance_main(t_initial);
//
//                if (GMAL_IS_SOURCE_PROGRAM(program))
//                    return GMAL_SOURCE_PROGRAM;
//            }
//        } else {
//            hash_set_destroy(s_top->done_set);
//            hash_set_destroy(s_top->backtrack_set);
//            puts("**** ... Nothing to backtrack on... ****");
//        }
//        csystem_pop_program_stacks_for_backtracking(&csystem);
//        printf("**** Backtracking completed at depth %d ****\n", depth);
//    }
    return false;
}

void
gmal_exit()
{
    puts("EXITING");
}

void*
gmal_create_shared_memory_region()
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
    int rc = ftruncate(fd, shmAllocationSize);
    if (rc == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    // We want stack at same address for each process.  Otherwise, a pointer
    //   to an address in the stack data structure will not work everywhere.
    //   Hopefully, this address is not already used.
    void *stack_address = (void *)0x4444000;
    void *shmStart = mmap(stack_address, shmAllocationSize,
                                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0);
    if (shmStart == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    // shm_unlink(dpor); // Don't unlink while child processes need to open this.
    fsync(fd);
    close(fd);
    return shmStart;
}

void
gmal_initialize_shared_memory_region()
{
    void *shm = gmal_create_shared_memory_region();
    void *threadQueueStart = shm;
    void *shmTransitionStart = (char*)threadQueueStart + sizeof(*threadQueue);
    shmStart = shm;
    threadQueue = static_cast<typeof(threadQueue)>(threadQueueStart);
    shmTransitionData = static_cast<typeof(shmTransitionData)>(shmTransitionStart);
}

void
gmal_create_thread_sleep_points()
{
    for (int i = 0; i < MAX_TOTAL_THREADS_IN_PROGRAM; i++)
        mc_shared_cv_init(&(*threadQueue)[i]);
}

void
gmal_reset_cv_locks()
{
    for (int i = 0; i < MAX_TOTAL_THREADS_IN_PROGRAM; i++) {
        mc_shared_cv_destroy(&(*threadQueue)[i]);
        mc_shared_cv_init(&(*threadQueue)[i]);
    }
}

void
sigusr1_handler(int sig)
{
    printf("******* CHILD EXITING with pid %lu *******************\n", (uint64_t)getpid());
    _Exit(0);
}

GMAL_PROGRAM_TYPE
gmal_spawn_child()
{
    // Ensure that a child does not already exist to prevent fork bombing
    GMAL_ASSERT(cpid == -1);

    pid_t childpid;
    if ( (childpid = fork()) < 0) {
        perror("fork");
        abort();
    }
    cpid = childpid;

    if (FORK_IS_CHILD_PID(childpid)) {
        signal(SIGUSR1, &sigusr1_handler);
        printf("*** CHILD SPAWNED WITH PID %lu***\n", (uint64_t)getpid());
        return GMAL_SOURCE_PROGRAM;
    }
    return GMAL_SCHEDULER;
}

GMAL_PROGRAM_TYPE
gmal_spawn_child_following_transition_stack()
{
//    gmal_reset_cv_locks();
//    GMAL_PROGRAM_TYPE program = gmal_begin_target_program_at_main();
//
//    if (GMAL_IS_SCHEDULER(program)) {
//        int transition_stack_height = csystem_transition_stack_count(&csystem);
//        for (int i = 0; i < transition_stack_height; i++) {
//            // NOTE: This is reliant on the fact
//            // that threads are created in the same order
//            // when we create them. This will always be consistent,
//            // but we might need to look out for when a thread dies
//
//            transition_ref t = csystem_transition_stack_get_element(&csystem, i);
////            printf("Respawn step %d. Running thread with id %lu\n", i + 1, t->thread.tid);
//            gmal_run_thread_to_next_visible_operation(t->thread.tid);
//        }
//    } else {
//        // We need to reset the concurrent system
//        // for the child since, at the time this method
//        // is invoked, it will have a complete copy of
//        // the state the of system. But we need to
//        // re-simulate the system by running the transitions
//        // in the transition stack; otherwise, shadow resource
//        // allocations will be off
//        csystem_reset(&csystem);
//        gmal_register_main_thread();
//    }
//
//    return program;
    return GMAL_SCHEDULER;
}

GMAL_PROGRAM_TYPE
gmal_begin_target_program_at_main()
{
    GMAL_PROGRAM_TYPE program = gmal_spawn_child();
    if (GMAL_IS_SOURCE_PROGRAM(program)) {
        // NOTE: Technically, the child will be frozen
        // inside of dpor_init until it is scheduled. But
        // this is only a technicality: it doesn't actually
        // matter where the child spawns so long as it reaches
        // the actual source program
//        tid_self = 0;

        gmal_initialize_shared_memory_region();
        thread_await_gmal_scheduler_for_thread_start_transition();
    }
    return program;
}

void
gmal_run_thread_to_next_visible_operation(tid_t tid)
{
    mc_assert(tid != TID_INVALID);
    mc_shared_cv_ref cv = &(*threadQueue)[tid];
    mc_shared_cv_wake_thread(cv);
    mc_shared_cv_wait_for_thread(cv);
}

void
gmal_child_kill()
{
    if (cpid == -1) return; // No child
    kill(cpid, SIGUSR1);
    waitpid(cpid, NULL, 0);
    cpid = -1;
}

void
gmal_exhaust_threads()
{
//    puts("**** Exhausting threads... ****");
//    int debug_depth = csystem_transition_stack_count(&csystem);
//    dynamic_transition_ref t_next = initial_transition;
//    do {
//        debug_depth++;
////        printf("**** At depth %d ****\n", debug_depth);
//        tid_t tid = t_next->thread->tid;
//        dpor_run_thread_to_next_visible_operation(tid);
//        csystem_simulate_running_thread(&csystem, shm_child_result, t_next->thread);
//        csystem_dynamically_update_backtrack_sets(&csystem);
//    } while ( (t_next = csystem_get_first_enabled_transition(&csystem)) != NULL );
//
//    // TODO: Test for deadlock
//    if (csystem_is_in_deadlock(&csystem)) {
//        puts("*** DEADLOCK DETECTED ***");
//        csystem_print_transition_stack(&csystem);
//        csystem_print_next_transitions_stack(&csystem);
//    } else {
//        puts("*** NO FAILURE DETECTED ***");
////        csystem_print_transition_stack(&csystem);
//    }
//    dpor_child_kill();
}

GMAL_PROGRAM_TYPE
gmal_readvance_main()
{
//    dpor_reset_cv_locks();
//    bool is_child = dpor_spawn_child_following_transition_stack();
//    if (is_child) return true;
//    dpor_exhaust_threads(initial_transition);
    return GMAL_SCHEDULER;
}


// NOTE: Assumes that the parent process
// is asleep (called dpor_run_thread_to_next_visible_operation); the behavior
// is undefined otherwise
void
thread_await_gmal_scheduler()
{
    mc_assert(tid_self != TID_INVALID);
    mc_shared_cv_ref cv = &(*threadQueue)[tid_self];
    mc_shared_cv_wake_scheduler(cv);
    mc_shared_cv_wait_for_scheduler(cv);
}

// NOTE: This should only be called in one location:
// When the scheduler starts, there is an initial
// race condition between the child process and the
// parent process with `thread_await_dpor_scheduler`. `thread_await_dpor_scheduler` assumes
// the scheduler (parent) process is asleep; but upon
// initialization this is not true. Hence, this method is invoked instead
void
thread_await_gmal_scheduler_for_thread_start_transition()
{
    mc_assert(tid_self != TID_INVALID);
    mc_shared_cv_ref cv = &(*threadQueue)[tid_self];
    mc_shared_cv_wait_for_scheduler(cv);
}

void
thread_awake_gmal_scheduler_for_thread_finish_transition()
{
    mc_assert(tid_self != TID_INVALID);
    mc_shared_cv_ref cv = &(*threadQueue)[tid_self];
    mc_shared_cv_wake_scheduler(cv);
}

tid_t
gmal_register_thread()
{
    tid_t newTid = programState.createNewThread();
    tid_self = newTid;
    return newTid;
}

tid_t
gmal_register_main_thread()
{
    tid_t newTid = programState.createMainThread();
    tid_self = newTid;
    return newTid;
}

// ****** CHILD FUNCTIONS END **** //
