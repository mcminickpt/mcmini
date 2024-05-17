#include "mcmini/mcmini_private.h"
#include "mcmini/MCSharedTransition.h"
#include "mcmini/MCTransitionFactory.h"
#include "mcmini/signals.h"
#include "mcmini/transitions/MCTransitionsShared.h"
#include <vector>
#include <sys/wait.h> // For waitpid
#include <errno.h>    // For errno
#include <cstring>    // For strerror
#include <iostream>   // For std::cerr

extern "C" {
#include "mcmini/mc_shared_sem.h"
#include <cassert>
#include <cstdio>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <ucontext.h>
}

using namespace std;

MC_THREAD_LOCAL tid_t tid_self = TID_INVALID;
pid_t trace_pid                = -1;

trid_t traceId      = 0;
trid_t transitionId = 0;

time_t mcmini_start_time = 0;

/**
 * The process id of the scheduler
 */
pid_t scheduler_pid = -1;
mc_shared_sem (*trace_sleep_list)[MAX_TOTAL_THREADS_IN_PROGRAM] =
  nullptr;
sem_t mc_pthread_create_binary_sem;

static char resultString[1000] = "***** Model checking completed! *****\n";
static void addResult(const char *result) {
  char stats[1000];
  if (strstr(resultString, result) != NULL) {
    result = "  (Other trace numbers (traceId) of bugs exist above;\n"
             "   Use --first-deadlock (-f) to only show first one)\n";
    if (strstr(resultString, result) == NULL) {
      strncat(resultString, result, sizeof(resultString) - strlen(resultString));
    }
    return;
  }
  strncat(resultString, result, sizeof(resultString) - strlen(resultString));
  snprintf(stats, 80, "  (Trace number (traceId): %lu)\n", traceId);
  strncat(resultString, stats, sizeof(resultString) - strlen(resultString));
}
static void printResults() {
  mcprintf(resultString);
  mcprintf("Number of traces: %lu\n", traceId);
  mcprintf("Total number of transitions: %lu\n", transitionId);
  mcprintf("Elapsed time: %lu seconds\n", time(NULL) - mcmini_start_time);
}

/*
 * Dynamically updated to control how McMini proceeds with its
 * execution.
 *
 * If at any point during the (re-)execution of the trace McMini
 * realizes that the current trace is no longer interesting it will
 * continue searching new traces as before until either the state
 * space has been exhausted or possibly McMini stops again to
 * re-execute a future trace
 *
 * NOTE: Misusing this variable could cause all sorts of bad stuff to
 * happen. McMini relies on this value to determine if it should stop
 * execution in the middle of regenerating a trace.
 *
 * TODO: We should perhaps figure out a way to absorb this information
 * into the debugger, or perhaps make it a local variable.
 * Effectively, we should make it more difficult to set this value
 * since it affects the behavior of a rather important function for
 * state regeneration (viz. mc_fork_next_trace_at_current_state())
 */
bool rerunCurrentTraceForDebugger = false;

/* Data transfer */
void *shmStart                            = nullptr;
MCSharedTransition *shmTransitionTypeInfo = nullptr;
void *shmTransitionData                   = nullptr;
const size_t shmAllocationSize =
  sizeof(*trace_sleep_list) +
  (sizeof(*shmTransitionTypeInfo) + MAX_SHARED_MEMORY_ALLOCATION);

/* Program state */
MCDeferred<MCStack> programState;

void
alarm_handler(int sig)
{
  if (sig == SIGALRM) {
    fprintf(stderr,
            "\n *** mcmini exiting after one hour.  To avoid, this,\n"
            " *** Use flag '--long-test' or  MCMINI_LONG_TEXT env. "
            "var.\n\n");
    _exit(1); // Note:  McMini wraps 'exit()'.  So, we use '_exit()'.
  }
}

ucontext_t mcmini_scheduler_main_context;

MC_CONSTRUCTOR void
mcmini_main()
{
  mcmini_start_time = time(NULL);

  getcontext(&mcmini_scheduler_main_context);

  if (getenv("MCMINI_PROCESS") == NULL) {
    setenv("MCMINI_PROCESS", "SCHEDULER", 1); // This is McMini scheduler proc
  } else { // Else this is McMini target process
    unsetenv("MCMINI_PROCESS");
    // Avoid polluting environment of target.  Or can set to "TARGET".
    return;
  }

  if (getenv(ENV_LONG_TEST) == NULL) {
    alarm(3600); // one hour
    signal(SIGALRM, alarm_handler);
  }
  mc_load_intercepted_symbol_addresses();
  mc_create_global_state_object();
  mc_initialize_shared_memory_globals();
  mc_initialize_trace_sleep_list();
  install_sighandles_for_scheduler();

  // Mark this process as the scheduler
  scheduler_pid = getpid();
  MC_FATAL_ON_FAIL(
    __real_sem_init(&mc_pthread_create_binary_sem, 0, 0) == 0);

  mc_do_model_checking();

  printResults();
  mc_stop_model_checking(EXIT_SUCCESS);
}

void
mc_create_global_state_object()
{
  auto config = get_config_for_execution_environment();
  programState.Construct(config);
  programState->registerVisibleOperationType(typeid(MCThreadStart),
                                             &MCReadThreadStart);
  programState->registerVisibleOperationType(typeid(MCThreadCreate),
                                             &MCReadThreadCreate);
  programState->registerVisibleOperationType(typeid(MCThreadFinish),
                                             &MCReadThreadFinish);
  programState->registerVisibleOperationType(typeid(MCThreadJoin),
                                             &MCReadThreadJoin);
  programState->registerVisibleOperationType(typeid(MCMutexInit),
                                             &MCReadMutexInit);
  programState->registerVisibleOperationType(typeid(MCMutexUnlock),
                                             &MCReadMutexUnlock);
  programState->registerVisibleOperationType(typeid(MCMutexLock),
                                             &MCReadMutexLock);
  programState->registerVisibleOperationType(typeid(MCSemInit),
                                             &MCReadSemInit);
  programState->registerVisibleOperationType(typeid(MCSemPost),
                                             &MCReadSemPost);
  programState->registerVisibleOperationType(typeid(MCSemWait),
                                             &MCReadSemWait);
  programState->registerVisibleOperationType(typeid(MCSemEnqueue),
                                             &MCReadSemEnqueue);
  programState->registerVisibleOperationType(typeid(MCExitTransition),
                                             &MCReadExitTransition);
  programState->registerVisibleOperationType(
    typeid(MCAbortTransition), &MCReadAbortTransition);
  programState->registerVisibleOperationType(typeid(MCBarrierEnqueue),
                                             &MCReadBarrierEnqueue);
  programState->registerVisibleOperationType(typeid(MCBarrierInit),
                                             &MCReadBarrierInit);
  programState->registerVisibleOperationType(typeid(MCBarrierWait),
                                             &MCReadBarrierWait);
  programState->registerVisibleOperationType(typeid(MCCondInit),
                                             &MCReadCondInit);
  programState->registerVisibleOperationType(typeid(MCCondSignal),
                                             &MCReadCondSignal);
  programState->registerVisibleOperationType(typeid(MCCondBroadcast),
                                             &MCReadCondBroadcast);
  programState->registerVisibleOperationType(typeid(MCCondWait),
                                             &MCReadCondWait);
  programState->registerVisibleOperationType(typeid(MCCondEnqueue),
                                             &MCReadCondEnqueue);
  programState->registerVisibleOperationType(typeid(MCRWLockInit),
                                             &MCReadRWLockInit);
  programState->registerVisibleOperationType(
    typeid(MCRWLockReaderEnqueue), &MCReadRWLockReaderEnqueue);
  programState->registerVisibleOperationType(
    typeid(MCRWLockWriterEnqueue), &MCReadRWLockWriterEnqueue);
  programState->registerVisibleOperationType(
    typeid(MCRWLockWriterLock), &MCReadRWLockWriterLock);
  programState->registerVisibleOperationType(
    typeid(MCRWLockReaderLock), &MCReadRWLockReaderLock);
  programState->registerVisibleOperationType(typeid(MCRWLockUnlock),
                                             &MCReadRWLockUnlock);

  programState->registerVisibleOperationType(typeid(MCRWWLockInit),
                                             &MCReadRWWLockInit);
  programState->registerVisibleOperationType(
    typeid(MCRWWLockReaderEnqueue), &MCReadRWWLockReaderEnqueue);
  programState->registerVisibleOperationType(
    typeid(MCRWWLockReaderLock), &MCReadRWWLockReaderLock);
  programState->registerVisibleOperationType(
    typeid(MCRWWLockWriter1Enqueue), &MCReadRWWLockWriter1Enqueue);
  programState->registerVisibleOperationType(
    typeid(MCRWWLockWriter1Lock), &MCReadRWWLockWriter1Lock);
  programState->registerVisibleOperationType(
    typeid(MCRWWLockWriter2Enqueue), &MCReadRWWLockWriter2Enqueue);
  programState->registerVisibleOperationType(
    typeid(MCRWWLockWriter2Lock), &MCReadRWWLockWriter2Lock);
  programState->registerVisibleOperationType(typeid(MCRWWLockUnlock),
                                             &MCReadRWWLockUnlock);
  programState->registerVisibleOperationType(
    typeid(MCGlobalVariableRead), &MCReadGlobalRead);
  programState->registerVisibleOperationType(
    typeid(MCGlobalVariableWrite), &MCReadGlobalWrite);
  programState->start();
}

int countVisibleObjectsOfType(int objectId) {
  int count = 0;
  for (int i = 0; i <= objectId; i++) {
    if (typeid(*(programState->getObjectWithId(i))) ==
        typeid(*(programState->getObjectWithId(objectId)))) {
      count++;
    }
  }
  return count;
}

void
mc_prepare_to_model_check_new_program()
{
  mc_register_main_thread();
  auto mainThread = programState->getThreadWithId(TID_MAIN_THREAD);
  auto initialTransition =
    MCTransitionFactory::createInitialTransitionForThread(mainThread);
  programState->setNextTransitionForThread(TID_MAIN_THREAD,
                                           initialTransition);
}

int
mc_explore_branch(int curBranchPoint)
{
  tid_t backtrackThread;

  if (curBranchPoint == FIRST_BRANCH) {
    mc_fork_new_trace();
    backtrackThread = TID_MAIN_THREAD;
  } else { // else next branch
    auto *sNext = &(programState->getStateItemAtIndex(curBranchPoint));
    backtrackThread = sNext->popThreadToBacktrackOn();

    // Prepare the scheduler's model of the next trace
    programState->reflectStateAtTransitionIndex(curBranchPoint - 1);

    mc_fork_next_trace_at_current_state();
  }

  mc_search_dpor_branch_with_thread(backtrackThread);
  mc_exit_with_trace_if_necessary(traceId);
  mc_run_next_trace_for_debugger();

  traceId++;
  if (traceId % 1000 == 0) {
    static time_t last_time_reported = mcmini_start_time;
    if (time(NULL) - last_time_reported > 10) {
      last_time_reported = time(NULL);
      mcprintf("... %d traces analyzed so far ...\n", traceId);
    }
  }
  return programState->getDeepestDPORBranchPoint();
}

void
mc_do_model_checking()
{
  mc_prepare_to_model_check_new_program();

  int nextBranchPoint = mc_explore_branch(FIRST_BRANCH);
  while (nextBranchPoint != FIRST_BRANCH) { // while not backtracked to origin
    nextBranchPoint = mc_explore_branch(nextBranchPoint);
    nextBranchPoint = programState->getDeepestDPORBranchPoint();
  }
}

void
mc_get_shm_handle_name(char *dst, size_t sz)
{
  snprintf(dst, sz, "/mcmini-%s-%lu", getenv("USER"), (long)getpid());
  dst[sz - 1] = '\0';
}

void *
mc_allocate_shared_memory_region()
{
  //  If the region exists, then this returns a fd for the existing
  //  region. Otherwise, it creates a new shared memory region.
  char dpor[100];
  mc_get_shm_handle_name(dpor, sizeof(dpor));

  // This creates a file in /dev/shm/
  int fd = shm_open(dpor, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    if (errno == EACCES) {
      fprintf(stderr,
              "Shared memory region '%s' not owned by this process\n",
              dpor);
    } else {
      perror("shm_open");
    }
    mc_exit(EXIT_FAILURE);
  }
  int rc = ftruncate(fd, shmAllocationSize);
  if (rc == -1) {
    perror("ftruncate");
    mc_exit(EXIT_FAILURE);
  }
  // We want stack at same address for each process.  Otherwise, a
  // pointer
  //   to an address in the stack data structure will not work
  //   everywhere. Hopefully, this address is not already used.
  void *stack_address = (void *)0x4444000;
  void *shmStart =
    mmap(stack_address, shmAllocationSize, PROT_READ | PROT_WRITE,
         MAP_SHARED | MAP_FIXED, fd, 0);
  if (shmStart == MAP_FAILED) {
    perror("mmap");
    mc_exit(EXIT_FAILURE);
  }
  // shm_unlink(dpor); // Don't unlink while child processes need to
  // open this.
  fsync(fd);
  close(fd);
  return shmStart;
}

void
mc_deallocate_shared_memory_region()
{
  char shm_file_name[100];
  mc_get_shm_handle_name(shm_file_name, sizeof(shm_file_name));
  int rc = munmap(shmStart, shmAllocationSize);
  if (rc == -1) {
    perror("munmap");
    mc_exit(EXIT_FAILURE);
  }

  rc = shm_unlink(shm_file_name);
  if (rc == -1) {
    if (errno == EACCES) {
      fprintf(stderr,
              "Shared memory region '%s' not owned by this process\n",
              shm_file_name);
    } else {
      perror("shm_unlink");
    }
    mc_exit(EXIT_FAILURE);
  }
}

void
mc_initialize_shared_memory_globals()
{
  void *shm              = mc_allocate_shared_memory_region();
  void *threadQueueStart = shm;
  void *shmTransitionTypeInfoStart =
    (char *)threadQueueStart + sizeof(*trace_sleep_list);
  void *shmTransitionDataStart = (char *)shmTransitionTypeInfoStart +
                                 sizeof(*shmTransitionTypeInfo);

  shmStart = shm;
  trace_sleep_list =
    static_cast<typeof(trace_sleep_list)>(threadQueueStart);
  shmTransitionTypeInfo = static_cast<typeof(shmTransitionTypeInfo)>(
    shmTransitionTypeInfoStart);
  shmTransitionData = shmTransitionDataStart;
}

void
mc_initialize_trace_sleep_list()
{
  for (unsigned int i = 0; i < MAX_TOTAL_THREADS_IN_PROGRAM; i++)
    mc_shared_sem_init(&(*trace_sleep_list)[i]);
}

void
mc_reset_cv_locks()
{
  for (unsigned int i = 0; i < MAX_TOTAL_THREADS_IN_PROGRAM; i++) {
    mc_shared_sem_destroy(&(*trace_sleep_list)[i]);
    mc_shared_sem_init(&(*trace_sleep_list)[i]);
  }
}

void
mc_fork_new_trace()
{
  // Ensure that a child does not already
  // exist to prevent fork bombing
  MC_ASSERT(trace_pid == -1);

  pid_t childpid;
  if ((childpid = fork()) < 0) {
    perror("fork");
    abort();
  }
  trace_pid = childpid;

  if (FORK_IS_CHILD_PID(childpid)) {
    prctl(PR_SET_PDEATHSIG, SIGUSR1, 0, 0); // In McMini, SIGUSR1 to kill child
    if (getenv(ENV_QUIET) != NULL) {
      close(0); assert(open("/dev/null", O_RDONLY) == 0);
      close(1); assert(open("/dev/null", O_WRONLY) == 1);
      close(2); assert(open("/dev/null", O_WRONLY) == 2);
    }

    install_sighandles_for_trace();

    // We need to reset the concurrent system
    // for the child since, at the time this method
    // is invoked, it will have a complete copy of
    // the state the of system. But we need to
    // re-simulate the system by running the transitions
    // in the transition stack; otherwise, shadow resource
    // allocations will be off
    programState->reset();
    programState->start();
    mc_register_main_thread();

    // NOTE: Technically, the child will be frozen
    // inside of dpor_init until it is scheduled. But
    // this is only a technicality: it doesn't actually
    // matter where the child spawns so long as it reaches
    // the actual source program
    tid_self = 0;

    // Note that the child process does
    // not need to re-map the shared memory
    // region as the parent has already done that

    // This is important to handle the case when the
    // main thread hits return 0; in that case, we
    // keep the process alive to allow the model checker to
    // continue working
    //
    // NOTE!!: atexit handlers can be invoked when a dynamic
    // library is unloaded. In the transparent target, we need
    // to be able to handle this case gracefully
    MC_FATAL_ON_FAIL(atexit(&mc_exit_main_thread) == 0);

    thread_await_scheduler_for_thread_start_transition();

    setcontext(&mcmini_scheduler_main_context);
  }
}

void
mc_fork_next_trace_at_current_state()
{
  mc_reset_cv_locks();
  mc_fork_new_trace();

  const int tStackHeight = programState->getTransitionStackSize();

  for (int i = 0; i < tStackHeight; i++) {
    // If we're currently working with a debugger and we want to
    // move to a different point in the trace, we can simply break
    // out of the loop moving the execution forward of the current
    // trace in order to start a new one.
    if (rerunCurrentTraceForDebugger) {
      break;
    }

    // NOTE: This is reliant on the fact
    // that threads are created in the same order
    // when we create them. This will always be consistent,
    // but we might need to look out for when a thread dies
    tid_t nextTid = programState->getThreadRunningTransitionAtIndex(i);
    mc_run_thread_to_next_visible_operation(nextTid);
  }
}

void mc_run_thread_to_next_visible_operation(tid_t tid) {
  MC_ASSERT(tid != TID_INVALID);
  mc_shared_sem_ref sem = &(*trace_sleep_list)[tid];
  mc_shared_sem_wake_thread(sem);
  mc_shared_sem_wait_for_thread(sem);
}

void mc_terminate_trace() {
  if (trace_pid == -1) return;  // No child
  kill(trace_pid, SIGUSR1);
  mc_wait_for_trace();
  trace_pid = -1;
}

void mc_wait_for_trace() {
  MC_ASSERT(trace_pid != -1);

  int status;
  char *v = getenv(ENV_VERBOSE);
  bool verbose = v ? v[0] == '1' : false;
  if (waitpid(trace_pid, &status, 0) == -1) {
    if (verbose) {
      fprintf(stderr, "Error waiting for trace process with pid `%lu` %s\n",
              (uint64_t)trace_pid, strerror(errno));
    }
  } else if (verbose) {
    // Check how the trace process exited
    if (WIFEXITED(status)) {
      fprintf(stderr,
              "Trace process with traceId `%lu` exited with status %d\n",
              traceId, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      fprintf(stderr,
              "Trace process with traceId `%lu` was killed by signal `%d`\n",
              traceId, WTERMSIG(status));
    } else {
      fprintf(stderr, "Trace process with traceId `%lu` exited abnormally.\n",
              traceId);
    }
  }
}

void mc_trace_panic() {
  pid_t schedpid = getppid();
  kill(schedpid, SIGUSR1);

  // The scheduler will kill the child
  // process before being able to leave this function
  waitpid(schedpid, nullptr, 0);
}

void
mc_search_dpor_branch_with_thread(const tid_t backtrackThread)
{
  uint64_t depth = programState->getTransitionStackSize();
  const MCTransition &initialTransition =
    programState->getNextTransitionForThread(backtrackThread);
  const MCTransition *nextTransition = &initialTransition;

  // TODO: Assert whether or not nextTransition is enabled
  // TODO: Assert whether a trace process exists at this point

  do {
    if (depth >= MAX_TOTAL_TRANSITIONS_IN_PROGRAM) {
      printResults();
      mcprintf(
        "*** Execution Limit Reached! ***\n\n"
        "McMini ran a trace with %lu transitions which is\n"
        "the most McMini can currently handle in any one trace. Try\n"
        "running mcmini with the \"--max-depth-per-thread\" flag\n"
        "to limit how far into a trace McMini can go\n",
        depth);
      mc_stop_model_checking(EXIT_FAILURE);
    }

    depth++;
    transitionId++;

    const tid_t tid = nextTransition->getThreadId();
    mc_run_thread_to_next_visible_operation(tid);

    programState->simulateRunningTransition(
      *nextTransition, shmTransitionTypeInfo, shmTransitionData);
    programState->dynamicallyUpdateBacktrackSets();

    /* Check for data races */
    {
      const MCTransition &nextTransitionForTid =
        programState->getNextTransitionForThread(tid);
      if (programState->hasADataRaceWithNewTransition(
            nextTransitionForTid)) {
        mcprintf("*** DATA RACE DETECTED ***\n");
        programState->printTransitionStack();
        programState->printNextTransitions();
        addResult("*** DATA RACE DETECTED ***\n");
      }
    }

    nextTransition = programState->getFirstEnabledTransition();
  } while (nextTransition != nullptr);

  const bool hasDeadlock        = programState->isInDeadlock();
  const bool programHasNoErrors = !hasDeadlock;

  if (hasDeadlock) {
    mcprintf("TraceId %lu, *** DEADLOCK DETECTED ***\n", traceId);
    programState->printTransitionStack();
    programState->printNextTransitions();
    addResult("*** DEADLOCK DETECTED ***\n");

    if (getenv(ENV_FIRST_DEADLOCK) != NULL) {
      printResults();
      mc_exit(EXIT_SUCCESS);
    }
  }

  static char *verbose = getenv(ENV_VERBOSE);
  if (programHasNoErrors && verbose) {
    if (verbose[0] == '1') {
      mcprintf("TraceId %3d:  ", traceId);
      programState->printThreadSchedule();
    } else {
      mcprintf("TraceId: %d, *** NO FAILURE DETECTED ***\n", traceId);
      programState->printTransitionStack();
      programState->printNextTransitions();
    }
  }

  mc_terminate_trace();
}

tid_t
mc_register_thread()
{
  tid_t newTid = programState->createNewThread();
  tid_self     = newTid;
  return newTid;
}

tid_t
mc_register_main_thread()
{
  tid_t newTid = programState->createMainThread();
  tid_self     = newTid;
  return newTid;
}

void
mc_report_undefined_behavior(const char *msg)
{
  mc_terminate_trace();
  fprintf(stderr,
          "\n"
          "Undefined Behavior Detected! \t\n"
          "............................ \t\n"
          "mcmini aborted the execution of trace with traceId %lu because\t\n"
          "it detected undefined behavior\t\n"
          "............................ \n"
          "Reason: %s \t\n\n",
          traceId, msg);
  programState->printTransitionStack();
  programState->printNextTransitions();
  mc_exit(EXIT_FAILURE);
}

/* GDB Interface */

void
mc_exit_with_trace_if_necessary(trid_t trid)
{
  if (programState->isTargetTraceIdForStackContents(trid)) {
    programState->printTransitionStack();
    programState->printNextTransitions();
    mc_stop_model_checking(EXIT_SUCCESS);
  }
}

void
mc_run_next_trace_for_debugger()
{
  while (rerunCurrentTraceForDebugger) {
    // McMini will only re-execute a trace
    // once. If a debugger wishes to trigger
    // McMini to execute more than once, it
    // could reset this value to `true`.
    rerunCurrentTraceForDebugger = false;
    mc_fork_next_trace_at_current_state();
    mc_terminate_trace();
  }
}

MCStackConfiguration
get_config_for_execution_environment()
{
  // FIXME: This is also in a bad spot. This needs to removed/changed
  // completely. We shouldn't need to pass arguments through the
  // environment. This suggests that mcmini would be better as a
  // single process that forks, exec()s w/LD_PRELOAD set, and then
  // remotely controls THAT process. We need to discuss this
  uint64_t maxThreadDepth = MC_STATE_CONFIG_THREAD_NO_LIMIT;
  trid_t gdbTraceNumber   = MC_STATE_CONFIG_NO_GDB_TRACE;
  trid_t stackContentDumpTraceNumber = MC_STAT_CONFIG_NO_TRANSITION_STACK_DUMP;
  bool firstDeadlock                  = false;
  bool expectForwardProgressOfThreads = false;

  // TODO: Sanitize arguments (check errors of strtoul)
  if (getenv(ENV_MAX_DEPTH_PER_THREAD) != NULL) {
    maxThreadDepth = strtoul(getenv(ENV_MAX_DEPTH_PER_THREAD), nullptr, 10);
  }

  if (getenv(ENV_DEBUG_AT_TRACE_ID) != NULL) {
    gdbTraceNumber = strtoul(getenv(ENV_DEBUG_AT_TRACE_ID), NULL, 10);
  }

  if (getenv(ENV_PRINT_AT_TRACE_ID) != NULL) {
    stackContentDumpTraceNumber =
      strtoul(getenv(ENV_PRINT_AT_TRACE_ID), nullptr, 10);
  }
  if (getenv(ENV_CHECK_FORWARD_PROGRESS) != NULL) {
    expectForwardProgressOfThreads = true;
  }

  if (getenv(ENV_FIRST_DEADLOCK) != NULL) {
    firstDeadlock = true;
  }

  return {maxThreadDepth, gdbTraceNumber, stackContentDumpTraceNumber,
          firstDeadlock, expectForwardProgressOfThreads};
}

bool
mc_is_scheduler()
{
  return scheduler_pid == getpid();
}

void
mc_exit(int status)
{
  // The exit() function is intercepted. Calling exit() directly
  // results in a deadlock since the thread calling it will block
  // forever (McMini does not let a process exit() during model
  // checking). Keep this in mind before switching this call to
  // a different exit function
  _Exit(status);
}

void
mc_stop_model_checking(int status)
{
  mc_deallocate_shared_memory_region();
  mc_terminate_trace();
  mc_exit(status);
}
