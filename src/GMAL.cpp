#include "GMAL.h"
#include "GMAL_Private.h"
#include "GMALSharedTransition.h"
#include "GMALTransitionFactory.h"
#include "transitions/GMALTransitionsShared.h"
#include <vector>

extern "C" {
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

/* The shmTransitionTypeInfo resides in shared memory */
/* The semaphores must also reside in shared memory as per the man page */

/* Synchronization primitives */
GMAL_THREAD_LOCAL tid_t tid_self = TID_INVALID;
pid_t cpid = -1;
mc_shared_cv (*threadQueue)[MAX_TOTAL_THREADS_IN_PROGRAM] = nullptr;
sem_t gmal_pthread_create_binary_sem;

/* Data transfer */
void *shmStart = nullptr;
GMALSharedTransition *shmTransitionTypeInfo = nullptr;
void *shmTransitionData = nullptr;
const size_t shmAllocationSize =  sizeof(*threadQueue) + (sizeof(*shmTransitionTypeInfo) + MAX_SHARED_MEMORY_ALLOCATION);

/* Program state */
GMALDeferred<GMALState> programState;

GMAL_CTOR void
gmal_init()
{
    gmal_load_shadow_routines();
    gmal_create_program_state();
    gmal_initialize_shared_memory_region();
    gmal_create_thread_sleep_points();

    GMAL_FATAL_ON_FAIL(__real_sem_init(&gmal_pthread_create_binary_sem, 0, 0) == 0);

    GMAL_PROGRAM_TYPE program = gmal_scheduler_main();
    if (GMAL_IS_SOURCE_PROGRAM(program)) return;

    puts("***** Model checking completed! *****");
    __real_exit(EXIT_SUCCESS);
}

void
gmal_create_program_state()
{
    auto config = get_config_for_execution_environment();
    programState.Construct(config);
    programState->registerVisibleOperationType(typeid(GMALThreadStart), &GMALReadThreadStart);
    programState->registerVisibleOperationType(typeid(GMALThreadCreate), &GMALReadThreadCreate);
    programState->registerVisibleOperationType(typeid(GMALThreadFinish), &GMALReadThreadFinish);
    programState->registerVisibleOperationType(typeid(GMALThreadJoin), &GMALReadThreadJoin);
    programState->registerVisibleOperationType(typeid(GMALMutexInit), &GMALReadMutexInit);
    programState->registerVisibleOperationType(typeid(GMALMutexUnlock), &GMALReadMutexUnlock);
    programState->registerVisibleOperationType(typeid(GMALMutexLock), &GMALReadMutexLock);
    programState->registerVisibleOperationType(typeid(GMALSemInit), &GMALReadSemInit);
    programState->registerVisibleOperationType(typeid(GMALSemPost), &GMALReadSemPost);
    programState->registerVisibleOperationType(typeid(GMALSemWait), &GMALReadSemWait);
    programState->registerVisibleOperationType(typeid(GMALExitTransition), &GMALReadExitTransition);
    programState->registerVisibleOperationType(typeid(GMALBarrierInit), &GMALReadBarrierInit);
    programState->registerVisibleOperationType(typeid(GMALBarrierWait), &GMALReadBarrierWait);
    programState->registerVisibleOperationType(typeid(GMALCondInit), &GMALReadCondInit);
    programState->registerVisibleOperationType(typeid(GMALCondSignal), &GMALReadCondSignal);
    programState->registerVisibleOperationType(typeid(GMALCondBroadcast), &GMALReadCondBroadcast);
    programState->registerVisibleOperationType(typeid(GMALCondWait), &GMALReadCondWait);
    programState->registerVisibleOperationType(typeid(GMALCondEnqueue), &GMALReadCondEnqueue);
    programState->start();
}

GMAL_PROGRAM_TYPE
gmal_scheduler_main()
{
    /*
     * Identifies the trace number of this call
     * Note that is we ever parallelized the program
     * this would be highly unsafe and would need to be atomic
     */
    static trid_t traceId = 0;

    gmal_register_main_thread();

    auto mainThread = programState->getThreadWithId(TID_MAIN_THREAD);
    auto initialTransition = GMALTransitionFactory::createInitialTransitionForThread(mainThread);
    programState->setNextTransitionForThread(TID_MAIN_THREAD, initialTransition);

    GMAL_PROGRAM_TYPE program = gmal_begin_target_program_at_main(false);
    if (GMAL_IS_SOURCE_PROGRAM(program))
        return GMAL_SOURCE_PROGRAM;

    gmal_exhaust_threads(initialTransition);
    program = gmal_enter_gdb_debugging_session_if_necessary(traceId++);
    if (GMAL_IS_SOURCE_PROGRAM(program))
        return GMAL_SOURCE_PROGRAM;

    int curStateStackDepth = static_cast<int>(programState->getStateStackSize());
    int curTransitionStackDepth = static_cast<int>(programState->getTransitionStackSize());

    while (curStateStackDepth > 0) {

        printf("**** Backtracking at state depth %d (%d transitions to reach this state) ****\n", curStateStackDepth, curTransitionStackDepth);

        std::shared_ptr<GMALStateStackItem> sTop = programState->getStateItemAtIndex(curStateStackDepth - 1);
        if (sTop->hasThreadsToBacktrackOn()) {
            // TODO: We can be smart here and only run a thread
            // if it is not already in a sleep set or lock set (eventually)
            programState->reflectStateAtTransitionDepth(curTransitionStackDepth - 1);

            // DPOR ensures that any thread in the backtrack set is enabled in this state
            tid_t backtrackThread = sTop->popFirstThreadToBacktrackOn();
            std::shared_ptr<GMALTransition> backtrackOperation = programState->getNextTransitionForThread(backtrackThread);

            program = gmal_enter_gdb_debugging_session_if_necessary(traceId++);
            if (GMAL_IS_SOURCE_PROGRAM(program))
                return GMAL_SOURCE_PROGRAM;

            program = gmal_readvance_main(backtrackOperation);
            if (GMAL_IS_SOURCE_PROGRAM(program))
                return GMAL_SOURCE_PROGRAM;

            curStateStackDepth = static_cast<int>(programState->getStateStackSize());
            curTransitionStackDepth = static_cast<int>(programState->getTransitionStackSize());
        } else {
            curStateStackDepth--;
            curTransitionStackDepth--;
            puts("**** ... Nothing to backtrack on... ****");
        }
        printf("**** Backtracking at state depth %d ****\n", curStateStackDepth);
    }
    return false;
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
        __real_exit(EXIT_FAILURE);
    }
    int rc = ftruncate(fd, shmAllocationSize);
    if (rc == -1) {
        perror("ftruncate");
        __real_exit(EXIT_FAILURE);
    }
    // We want stack at same address for each process.  Otherwise, a pointer
    //   to an address in the stack data structure will not work everywhere.
    //   Hopefully, this address is not already used.
    void *stack_address = (void *)0x4444000;
    void *shmStart = mmap(stack_address, shmAllocationSize,
                                        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0);
    if (shmStart == MAP_FAILED) {
        perror("mmap");
        __real_exit(EXIT_FAILURE);
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
    void *shmTransitionTypeInfoStart = (char*)threadQueueStart + sizeof(*threadQueue);
    void *shmTransitionDataStart = (char*)shmTransitionTypeInfoStart + sizeof(*shmTransitionTypeInfo);

    shmStart = shm;
    threadQueue = static_cast<typeof(threadQueue)>(threadQueueStart);
    shmTransitionTypeInfo = static_cast<typeof(shmTransitionTypeInfo)>(shmTransitionTypeInfoStart);
    shmTransitionData = shmTransitionDataStart;
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
sigusr1_handler_child(int sig)
{
    printf("******* CHILD EXITING with pid %lu *******************\n", (uint64_t)getpid());
    _Exit(0);
}

void
sigusr1_handler_scheduler(int sig)
{
    gmal_child_kill();
    puts("******* Something went wrong in the source program... *******************");
    _Exit(1);
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
        signal(SIGUSR1, &sigusr1_handler_child);
        printf("*** CHILD SPAWNED WITH PID %lu***\n", (uint64_t)getpid());
        return GMAL_SOURCE_PROGRAM;
    } else {
        GMAL_FATAL_ON_FAIL(signal(SIGUSR1, &sigusr1_handler_scheduler) != SIG_ERR);
        return GMAL_SCHEDULER;
    }
}

GMAL_PROGRAM_TYPE
gmal_spawn_child_following_transition_stack()
{
    gmal_reset_cv_locks();
    GMAL_PROGRAM_TYPE program = gmal_begin_target_program_at_main(false);

    if (GMAL_IS_SCHEDULER(program)) {
        const int transition_stack_height = programState->getTransitionStackSize();
        for (int i = 0; i < transition_stack_height; i++) {
            // NOTE: This is reliant on the fact
            // that threads are created in the same order
            // when we create them. This will always be consistent,
            // but we might need to look out for when a thread dies
            tid_t nextTid = programState->getThreadRunningTransitionAtIndex(i);
            gmal_run_thread_to_next_visible_operation(nextTid);
        }
    } else {
        // We need to reset the concurrent system
        // for the child since, at the time this method
        // is invoked, it will have a complete copy of
        // the state the of system. But we need to
        // re-simulate the system by running the transitions
        // in the transition stack; otherwise, shadow resource
        // allocations will be off
        programState->reset();
        gmal_register_main_thread();
    }

    return program;
}

GMAL_PROGRAM_TYPE
gmal_begin_target_program_at_main(bool spawnDaemonThread)
{
    GMAL_PROGRAM_TYPE program = gmal_spawn_child();
    if (GMAL_IS_SOURCE_PROGRAM(program)) {
        // NOTE: Technically, the child will be frozen
        // inside of dpor_init until it is scheduled. But
        // this is only a technicality: it doesn't actually
        // matter where the child spawns so long as it reaches
        // the actual source program
        tid_self = 0;
        gmal_initialize_shared_memory_region();

        // This is important to handle the case when the
        // main thread hits return 0; in that case, we
        // keep the process alive to allow the model checker to
        // continue working
        //
        //
        // NOTE: This does not handle the case where a
        // thread makes a call to exit(). This is a special case we need
        // to be able to handle
        //
        // NOTE!!: atexit handlers can be invoked when a dynamic
        // library is unloaded. In the transparent target, we need
        // to be able to handle this case gracefully
        GMAL_FATAL_ON_FAIL(atexit(&gmal_exit_main_thread) == 0);

        if (spawnDaemonThread)
            gmal_spawn_daemon_thread();

        thread_await_gmal_scheduler_for_thread_start_transition();
    }
    return program;
}

void
gmal_run_thread_to_next_visible_operation(tid_t tid)
{
    GMAL_ASSERT(tid != TID_INVALID);
    mc_shared_cv_ref cv = &(*threadQueue)[tid];
    mc_shared_cv_wake_thread(cv);
    mc_shared_cv_wait_for_thread(cv);
}

void
gmal_child_kill()
{
    if (cpid == -1) return; // No child
    kill(cpid, SIGUSR1);
    waitpid(cpid, nullptr, 0);
    cpid = -1;
}

void
gmal_child_wait()
{
    GMAL_ASSERT(cpid != -1);
    waitpid(cpid, nullptr, 0);
}

void
gmal_child_panic()
{
    pid_t schedpid = getppid();
    kill(schedpid, SIGUSR1);

    // The scheduler will kill the child
    // process before being able to leave this function
    waitpid(schedpid, nullptr, 0);
}

void
gmal_exhaust_threads(std::shared_ptr<GMALTransition> initialTransition)
{
    puts("**** Exhausting threads... ****");
    uint64_t debug_depth = programState->getTransitionStackSize();
    std::shared_ptr<GMALTransition> t_next = initialTransition;
    do {
        debug_depth++;
        tid_t tid = t_next->getThreadId();
        gmal_run_thread_to_next_visible_operation(tid);
        programState->simulateRunningTransition(t_next, shmTransitionTypeInfo, shmTransitionData);
        programState->dynamicallyUpdateBacktrackSets();
    } while ((t_next = programState->getFirstEnabledTransitionFromNextStack()) != nullptr);

    if (programState->programIsInDeadlock()) {
        puts("*** DEADLOCK DETECTED ***");
        programState->printTransitionStack();
        programState->printNextTransitions();
    } else {
        puts("*** NO FAILURE DETECTED ***");
    }
    gmal_child_kill();
}

GMAL_PROGRAM_TYPE
gmal_readvance_main(std::shared_ptr<GMALTransition> nextTransitionToTest)
{
    GMAL_PROGRAM_TYPE program = gmal_spawn_child_following_transition_stack();
    if (GMAL_IS_SOURCE_PROGRAM(program)) return GMAL_SOURCE_PROGRAM;
    gmal_exhaust_threads(nextTransitionToTest);
    return GMAL_SCHEDULER;
}

tid_t
gmal_register_thread()
{
    tid_t newTid = programState->createNewThread();
    tid_self = newTid;
    return newTid;
}

tid_t
gmal_register_main_thread()
{
    tid_t newTid = programState->createMainThread();
    tid_self = newTid;
    return newTid;
}

void
gmal_report_undefined_behavior(const char *msg)
{
    gmal_child_kill();
    fprintf(stderr,"\t Undefined Behavior Detected! \t\n"
            "\t ............................ \t\n"
            "\t The model checker aborted the execution because\t\n"
            "\t it detected undefined behavior\t\n"
            "\t ............................ \t\n"
            "\t %s \t\n",
    msg);
    programState->printTransitionStack();
    programState->printNextTransitions();
    exit(EXIT_FAILURE);
}

/* GDB Interface */

GMAL_PROGRAM_TYPE
gmal_enter_gdb_debugging_session_if_necessary(trid_t trid)
{
    if (gmal_should_enter_gdb_debugging_session_with_trace_id(trid))
        return gmal_enter_gdb_debugging_session();
    return GMAL_SCHEDULER;
}

bool
gmal_should_enter_gdb_debugging_session_with_trace_id(trid_t trid)
{
    return programState->isTargetTraceIdForGDB(trid);
}

GMAL_PROGRAM_TYPE
gmal_enter_gdb_debugging_session()
{
    GMAL_PROGRAM_TYPE program = gmal_begin_target_program_at_main(true);
    if (GMAL_IS_SCHEDULER(program)) {
        gmal_child_wait(); /* The daemon thread will take the place of the parent process */
        exit(0);
    }

    return program;
}

void
gmal_spawn_daemon_thread()
{
    auto trace = new std::vector<tid_t>();
    *trace = programState->getThreadIdTraceOfTransitionStack();

    pthread_t daemon;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    __real_pthread_create(&daemon, nullptr, &gmal_daemon_thread_simulate_program, trace);
    pthread_attr_destroy(&attr);
}

void*
gmal_daemon_thread_simulate_program(void *trace)
{
    programState->reset();
    programState->start();
    gmal_register_main_thread();

    auto mainThread = programState->getThreadWithId(TID_MAIN_THREAD);
    auto initialTransition = GMALTransitionFactory::createInitialTransitionForThread(mainThread);
    programState->setNextTransitionForThread(TID_MAIN_THREAD, initialTransition);

    auto tracePtr = static_cast<std::vector<tid_t>*>(trace);

    for (auto &tid : *tracePtr) {
        auto t_next = programState->getPendingTransitionForThread(tid);
        t_next->print();
        gmal_run_thread_to_next_visible_operation(tid);
        programState->simulateRunningTransition(t_next, shmTransitionTypeInfo, shmTransitionData);
    }
    delete tracePtr;
    gmal_child_panic();
    return nullptr; /* Ignored */
}

GMALStateConfiguration
get_config_for_execution_environment()
{
    uint64_t maxThreadDepth = GMAL_STATE_CONFIG_THREAD_NO_LIMIT;
    uint64_t gdbTraceNumber = GMAL_STATE_CONFIG_NO_TRACE;

    /* Parse the max thread depth from the command line (if available) */
    char *maxThreadDepthChar = getenv(ENV_THREAD_DEPTH);
    char *gdbTraceNumberChar = getenv(ENV_GDB_EXECUTION_TRACE);

    // TODO: Sanitize arguments (check errors of strtoul)
    if (maxThreadDepthChar != nullptr)
        maxThreadDepth = strtoul(maxThreadDepthChar, nullptr, 10);

    if (gdbTraceNumberChar)
        gdbTraceNumber = strtoul(gdbTraceNumberChar, nullptr, 10);

    return {maxThreadDepth, gdbTraceNumber};
}