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
 * symbols in the underlying thread libraries.
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
 * when new threads in each trace process are created, they must
 * assign themselves to the next available ID through the
 * `MCProgramState` instance available to that process (see the
 * `programState` global variable defined below). Thus, both trace
 * processes and the scheduler process manage creating thread IDs
 * separately in such a way that the scheduler has the absolute "view"
 * of all threads across traces while a particular trace understands
 * the thread IDs within a single branch of McMini
 */
extern MC_THREAD_LOCAL tid_t tid_self;

/*
 * Identifies the current trace being examined by McMini
 *
 * TODO: It would be better to have trace ids in the context
 * of a single execution of a program. McMini should theoretically be
 * able to model-check multiple programs in sequence
 *
 * NOTE: If we ever parallelized the program this would be highly
 * unsafe and would need to be atomic
 */
extern trid_t traceId;
extern pid_t trace_pid;

/**
 * @brief A fixed-size array assigning to each possible
 * thread of a McMini trace-process a location that at any given time
 * can receive notifications (and send notifications to) the
 * scheduler.
 *
 * When a thread in a trace process is created, it blocks on a
 * semaphore in shared memory uniquely pre-assigned to it contained
 * within this list. The thread ID assigned to it by McMini is treated
 * as an index into this list
 *
 * The list is of a fixed size to give the scheduler awareness of the
 * threads that it will have to deal with and, more importantly, so
 * that it can initialize the semaphores contained in the list to the
 * appropriate values
 *
 * FIXME: It *may* be possible to allow the list to have a variable
 * size. This would have to be communicated somehow back to the
 * scheduler, which would be complicated as the scheduler cannot know
 * a prior if new threads will be created. It's something to consider
 * in the future
 *
 * FIXME: Shared memory management should occur within a dedicated
 * object perhaps that wraps the memory and makes sure it is cleaned
 * up when the object is destroyed. Furthermore, we should de-couple
 * indexing the list with the thread ID and instead think of the
 * thread ID as a key in a map
 */
extern mc_shared_cv (*trace_sleep_list)[MAX_TOTAL_THREADS_IN_PROGRAM];

/**
 * @brief Initializes the variables in the global `trace_sleep_list`
 */
void mc_initialize_trace_sleep_list();

/**
 * @brief Destorys and then re-initializes the elements of
 * `trace_sleep_list`
 *
 * When the scheduler decides to explore a new trace, it must ensure
 * that the semaphores the threads in the *next* trace process will
 * access (which are the SAME as those used by the threads in the
 * previous trace) are initialized to their initial values. Otherwise
 * the scheduler and trace process may race with one another and will
 * invoke undefined behavior since the counts of the semaphores may
 * not correspond to that of a fresh trace having been created
 */
void mc_reset_trace_sleep_list();

/**
 * @brief A binary semaphore that is used to ensure that threads
 * created in trace processes have fully initialized before the
 * scheduler is notified of the completion of the thread creation
 * operation.
 *
 * When a thread in a trace process is scheduled to execute a function
 * that creates a *new* thread (e.g. pthread_create()), McMini blocks
 * (as with any transition it executes) and waits until thread
 * creation has completed.
 *
 * Thread creation is a bit of an edge case however. We must ensure
 * that the newly-spawned thread that is created has been assigned to
 * a thread ID BEFORE waking the scheduler; otherwise a race can
 * ensure whereby multiple spawned threads race to acquire thread IDs
 * within a trace. This can result in a mismatch between the IDs the
 * scheduler assigned to the threads and those the threads acquired
 * themselves (recall that threads acquire IDs via the current state,
 * which is private to both the trace AND the scheduler) since the
 * scheduler assigns higher thread IDs to threads created deeper in
 * the execution of the program.
 *
 * This semaphore is used to ensure that newly-spawned threads have
 * initialized themselves before the spawning thread wakes the
 * scheduler.
 */
extern sem_t mc_pthread_create_binary_sem;

/**
 * @brief The address at which the shared memory mailbox begins to
 * allow threads in a trace process to communicate with the scheduler
 */
extern void *shmStart;

/**
 * @brief The size of the shared memory allocation in bytes
 */
extern const size_t shmAllocationSize;

/**
 * @brief The address in shared memory at which information about the
 * *type* of the transition invoked by the thread in the trace process
 * last scheduled by the scheduler process is written into
 */
extern MCSharedTransition *shmTransitionTypeInfo;

/**
 * @brief The address in shared memory at which information about the
 * *payload* of the transition hit by the thread in the trace process
 * last scheduled by the scheduler process is written into
 */
extern void *shmTransitionData;

/**
 * @brief Allocates space for the shared memory mailbox used for
 * cross-process communication between forked trace processes and the
 * scheduler process
 *
 * @return void* The address of the shared memory allocated by this
 * function, or NULL if the memory could not be allocated
 */
void *mc_allocate_shared_memory_region();

/**
 * @brief Deallocates the space for the shared memory mailbox used for
 * cross-process communication between forked trace processed and the
 * scheduler process
 */
void mc_deallocate_shared_memory_region();

/**
 * @brief Initializes the global variables related to shared memory as
 * defined above
 */
void mc_initialize_shared_memory_globals();

/**
 * @brief A representation of the state of the current branch in the
 * state space explored by the scheduler under DPOR
 *
 * The program state is McMini's representation of the states of the
 * visible objects and threads of the target program at any given
 * point in time. As a trace process evolves and executes more
 * transitions, the state is updated to reflect this fact.
 */
extern MCDeferred<MCState> programState;

/**
 * @brief Initialize the global program state object `programState`
 *
 * FIXME: A better alternative is that we shouldn't need this function
 * and instead the global state is only locally accessible perhaps
 */
void mc_create_global_state_object();

/**
 * @brief Perform setup for the global program state object in
 * preparation for a new model checking session
 *
 * This method registers the main thread and marks that thread as
 * waiting to execute. Every program begins in a state with the main
 * thread suspended and about to execute the main routine.
 */
void mc_prepare_to_model_check_new_program();

/**
 * @brief Performs the model checking for the program McMini was
 * dynamically loaded into
 *
 * After the initialization phase, this function is invoked to perform
 * the actual model-checking using DPOR of the test program
 *
 * @return MC_PROGRAM_TYPE An identifier for which program exited the
 * call to the function. This gives the information callers need to
 * allow fork()-ed trace process to escape into the target program for
 * testing
 */
MC_PROGRAM_TYPE mc_do_model_checking();

/**
 * @brief Perform the first (depth-first) search of the state space
 * and fill the state with backtrack points for later searching
 *
 * From the perspective of any userspace program, every program begins
 * with the main thread about to enter the main function (ignoring any
 * processing the main thread actually does to its dynamic library
 * dependencies). This method prepares the `programState` object for
 * execution from the beginning of the program
 *
 * Executing the initial trace is not much different than executing
 * any other trace. The primary difference resides in the fact that
 * the state is set up to have only a single thread -- the main thread
 * in particular -- waiting to execute the "thread start" transition.
 *
 * @return MC_PROGRAM_TYPE An identifier for which program exited the
 * call to the function. This gives the information callers need to
 * allow a fork()-ed trace process to escape into the target program
 * for testing
 */
MC_PROGRAM_TYPE mc_run_initial_trace();

/**
 * @brief Begins searching a new branch in the state space starting
 * with "leadThread" executing from the current program state
 *
 * @param leadThread the thread whose execution should be followed
 * from the current program state to continue the DPOR, depth-first
 * search.
 *
 * @return The process identifier. The caller should allow
 * the process to escape into the target program as quickly
 * as possible to allow the scheduler to begin controlling its
 * execution
 */
MC_PROGRAM_TYPE
mc_search_next_dpor_branch_with_initial_thread(
  const tid_t leadThread);

/**
 * @brief Begins searching a new branch in the state space starting
 * with "leadThread" executing from the current program state
 *
 * This method makes two assumptions:
 *   1. There is a trace process forked and waiting for the scheduler
 *      to send requests for threads to execute through the mailbox
 *      (see the global variables residing in shared memory declared
 *      at the start of the header file).
 *   2. The thread that is about to execute is enabled in the given
 *      state.
 *
 * If either assumption is invalid, McMini will abort the backtracking
 * session; for otherwise McMini would be stuck in a deadlock.
 *
 * @param leadThread the thread whose execution should be followed
 * from the current program state to continue the DPOR, depth-first
 * search.
 */
void
mc_search_dpor_branch_with_initial_thread(const tid_t leadThread);

/* Source program management */
/*
 * FIXME: We should have a mcmini::ProcessVendor() here which manages
 * the lifetimes of processes. This may help simplify the logic and
 * will remove MC_PROGRAM_TYPE in favor of an enum of the same name
 */

/**
 * @brief Forks a new trace process whose execution begins immediately
 * after this function
 *
 * When a new trace process is created, its execution begins inside
 * the control flow of the scheduler that spawned it. As the trace is
 * responsible only for allowing processes
 *
 * Note that all global variables are effectively *duplicated* across
 * calls to this method (it eventually makes a call to fork(2)). Thus
 * modifications to all variables (not explicitly mapped into shared
 * memory) are isolated to the scheduler and each newly-spawned
 * trace
 *
 * @return The process identifier. The caller should allow
 * the process to escape into the target program as quickly
 * as possible to allow the scheduler to begin controlling its
 * execution
 */
MC_PROGRAM_TYPE mc_fork_new_trace();

/**
 * @brief Forks a new trace process whose execution is blocked until
 * the scheduler wakes it
 *
 * Conceptually, one can imagine that the trace is about to enter the
 * main routine of the target program. The newly created trace will be
 * blocked waiting to enter the main function until the scheduler
 * later allows the thread to exit
 *
 * @return The process identifier. The caller should allow
 * the process to escape into the target program as quickly
 * as possible
 */
MC_PROGRAM_TYPE mc_fork_new_trace_at_main();

/**
 * @brief Forks a new trace process whose run-time state reflects that
 * maintained by the scheduler in `programState`
 *
 * When the scheduler decides to explore a different branch of the
 * state space (i.e. if DPOR decides that some other branch needs to
 * be searched), it must regenerate a trace process whose "state"
 * (i.e. memory and threads) matches that originally recorded by the
 * scheduler at the particular point in the past (i.e. backtracking
 * point)
 *
 * @return The process identifier. The caller should allow
 * the process to escape into the target program as quickly
 * as possible
 */
MC_PROGRAM_TYPE mc_fork_new_trace_at_current_state();

/**
 * @brief Unblocks the thread corresponding to _tid_ in the current
 * trace process so it can execute a visible operation
 *
 * The caller will block until the thread in the trace process
 *
 * FIXME: We currently do not properly handle the case where the trace
 * process dies unexpectedly. Instead of deadlocking in the scheduler,
 * a more graceful approach would be to detect this with e.g. the
 * result of waitpid() in a separate thread spawned by McMini that
 * simply acts as a "watchdog" for bad events such as these
 *
 * @param tid the ID of the thread to allow to execute in the trace
 * process
 */
void mc_run_thread_to_next_visible_operation(tid_t tid);

/**
 * @brief Blocks execution of the calling thread until the current
 * trace process has fully exited
 */
void mc_wait_for_trace();

/**
 * @brief Halts the current trace and waits for it to exit
 */
void mc_terminate_trace();

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
 * The function blocks indefinitely and the calling process will later
 * be killed by the parent process.
 */
void mc_trace_panic();

/**
 * @brief Exit the program as if exit(2) were called
 *
 * To handle target programs which make a call to exit(2), McMini
 * intercepts the "exit" symbol and treats exit() as a visible
 * operation that is never enabled.
 *
 * The bigger problem is that McMini registers an at_exit() handler
 * that handles the case where the *main* thread exits the main
 * routine. Thus a call to exit(2), even through __real_exit() defined
 * by McMini, in the trace process results in the at_exit() handler
 * being invoked which causes a deadlock (since the at_exit() handler
 * merely acts as a direct call to exit(2) by the main thread).
 *
 * Hence, you should *always* call this method instead of directly
 * invoking exit(2) or __real_exit().
 */
void mc_exit(int);

/**
 * @brief Aborts model checking mid-exection
 *
 * McMini will abort execution and exit with the provided exit code.
 * Any shared memory allocated for cross-process communication will be
 * deallocated. If a trace process exists at the time the method is
 * executed, the trace process will first be killed
 *
 * @param status the exit code passed to the exit(2) system call
 */
void mc_stop_model_checking(int status);

/* Erroneous Behavior */

/**
 * @brief Stops model checking and reports that undefined behavior in
 * the current trace was encountered
 *
 * If a trace exhibits undefined behavior, McMini will abort model
 * checking and report the trace leading to undefined behavior. For
 * example, if the trace used an uninitialized mutex or semaphore it
 * would be invoking undefined behavior.
 *
 * McMini assumes that the behavior of the program is well-defined;
 * undefined behavior provides no guarantees about the behavior of the
 * underlying program and McMini makes no attempt to guess at it
 * (indeed it can't). The appropriate modifications to the program
 * must be made before model-checking is to begin again. (Otherwise
 * the same undefined behavior will repeatedly be encountered.)
 */
void mc_report_undefined_behavior(const char *);

/* Interactive Debugging Support */

/**
 * @brief Re-execute the current trace as many times as needed
 *
 * When this method is invoked, the current trace can be re-executed
 * an arbitrary number of times. By default, the function will not
 * cause any re-executions.
 *
 * @return The process identifier. The caller should allow
 * the process to escape into the target program as quickly
 * as possible
 */
MC_PROGRAM_TYPE mc_rerun_current_trace_as_needed();

MCStateConfiguration get_config_for_execution_environment();

/* Trace prints */
void mc_exit_with_trace_if_necessary(trid_t);

/* Registering and accessing threads */
tid_t mc_register_thread();
tid_t mc_register_main_thread();

#endif // INCLUDE_MCMINI_MCMINI_PRIVATE_HPP
