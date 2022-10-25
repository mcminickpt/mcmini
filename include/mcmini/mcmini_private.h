#ifndef INCLUDE_MCMINI_MCMINI_PRIVATE_HPP
#define INCLUDE_MCMINI_MCMINI_PRIVATE_HPP

#include "mcmini/MCDeferred.h"
#include "mcmini/MCShared.h"
#include "mcmini/MCSharedTransition.h"
#include "mcmini/MCState.h"
#include "mcmini/mcmini_wrappers.h"

extern "C" {
#include "mcmini/MCCommon.h"
#include "mcmini/MCEnv.h"
#include "mcmini/mc_shared_cv.h"
}

/**
 * @brief The entry point of McMini from which model checking begins
 *
 * Execution begins inside this function (marked as the constructor
 * for the dynamic library compiled from this source and called
 * directly with source code mixing McMini functions). Besides running
 * the DPOR engine itself, the function initializes several important
 * global variables managing the program state, shared memory, and
 * symbols in the underlying thread libraries
 */
MC_CTOR void mcmini_main();

/**
 * @brief The ID McMini uses to identify THIS thread, viz.
 * the thread that has access to this data
 *
 * Each new thread object (an instance of type MCThread) created
 * inside the scheduler is associated with a positive integer which is
 * used by the scheduler to identify threads executing in the trace
 * processes.
 *
 * Thread IDs are monotonically-increasing, with each subsequent
 * thread receiving an ID one greater than the previously created
 * thread.
 *
 * Since trace processes are ephemeral (they are discarded after the
 * scheduler has determined it can't execute the trace any further),
 * as new threads in each trace process are created, they must assign
 * themselves to the next available ID through the `MCProgramState`
 * instance available to that process (see the `programState` global
 * variable defined below). Thus, both trace processes and the
 * scheduler process manage creating thread IDs separately in such a
 * way that the scheduler has the absolute "view" of all threads
 * across traces while a particular trace understands the thread IDs
 * within a single branch of McMini
 */
extern MC_THREAD_LOCAL tid_t tid_self;

/**
 * @brief A fixed-size array assigning to each possible
 * trace-process thread McMini can support at any given time a loction
 * to receive notifications from a(nd send notifications to) the
 * scheduler.
 */
extern mc_shared_cv (
  *trace_sleep_queue)[MAX_TOTAL_THREADS_IN_PROGRAM];

/**
 * @brief A binary semaphore that is used to ensure that threads
 * created in trace processes have fully blocked before the scheduler
 * is notified of the thread creation operation completing
 *
 * When a thread in a trace process is scheduled to execute a function
 * which creates a *new* thread (e.g. pthread_create()), McMini
 */
extern sem_t mc_pthread_create_binary_sem;

/**
 * @brief
 *
 */
extern void *shmStart;

extern MCSharedTransition *shmTransitionTypeInfo;
extern void *shmTransitionData;
extern const size_t shmAllocationSize;

/**
 * @brief A representation of the state of the current trace process
 * in this DPOR state space branch
 *
 * The
 *
 */
extern MCDeferred<MCState> programState;

/**
 * @brief Initialize the global program state object
 *
 * FIXME: A better alternative is that we shouldn't need this function
 * and instead the global state is only locally accessible perhaps?
 *
 */
void mc_create_global_state_object();

/**
 * @brief Alerts the DPOR scheduler process an unrecoverable error
 * occurred while executing the trace
 *
 * When this function is called in a trace process, the trace delivers
 * a SIGUSR1 to the scheduler process (viz. the trace's parent), which
 * the scheduler is registered to respond to via signal(2) at
 * initialization-time of McMini (see `mcmini_main()` for more
 * details).
 *
 * The function blocks indefinitely and the process will later be
 * killed by the parent process
 */
void mc_child_panic();

/**
 * @brief
 *
 */
void mc_report_undefined_behavior(const char *);

#define MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(x, str)                \
  do {                                                              \
    if (!static_cast<bool>(x)) {                                    \
      mc_report_undefined_behavior(static_cast<const char *>(str)); \
    }                                                               \
  } while (0)

#define MC_REPORT_UNDEFINED_BEHAVIOR(str) \
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(false, str)

/* Scheduler control */
MC_PROGRAM_TYPE mc_scheduler_main();
void mc_exhaust_threads(const MCTransition &);
MC_PROGRAM_TYPE mc_readvance_main(const MCTransition &);
void mc_create_initial_scheduler_state();
void mc_exit(int);

/* GDB interface */
bool mc_should_enter_gdb_debugging_session_with_trace_id(trid_t);
MC_PROGRAM_TYPE mc_enter_gdb_debugging_session_if_necessary(trid_t);
MC_PROGRAM_TYPE mc_enter_gdb_debugging_session();
void mc_spawn_daemon_thread();
void *mc_daemon_thread_simulate_program(void *);
MCStateConfiguration get_config_for_execution_environment();

/* Trace prints */
void mc_exit_with_trace_if_necessary(trid_t);

/* Registering and accessing threads */
tid_t mc_register_thread();
tid_t mc_register_main_thread();

/* Shared memory management */
void *mc_create_shared_memory_region();
void mc_initialize_shared_memory_region();

void mc_initialize_trace_sleep_queue();
void mc_reset_thread_sleep_points();

/* Source program management */
MC_PROGRAM_TYPE mc_spawn_child();
MC_PROGRAM_TYPE mc_spawn_child_following_transition_stack();
MC_PROGRAM_TYPE
mc_begin_target_program_at_main(bool spawnDaemonThread);
void mc_run_thread_to_next_visible_operation(tid_t);
void mc_kill_child();
void mc_child_wait();

/* Thread control */
void thread_await_mc_scheduler();
void thread_await_mc_scheduler_for_thread_start_transition();
void thread_awake_mc_scheduler_for_thread_finish_transition();

#endif // INCLUDE_MCMINI_MCMINI_PRIVATE_HPP
