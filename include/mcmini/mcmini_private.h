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
#include <semaphore.h>
}

/* Synchronization primitives */
extern MC_THREAD_LOCAL tid_t tid_self;
extern pid_t cpid;
extern mc_shared_cv (*threadQueue)[MAX_TOTAL_THREADS_IN_PROGRAM];

/*
 * Allows new threads to be created in a race-free manner
 */
extern sem_t mc_pthread_create_binary_sem;
extern trid_t traceId;

/* Data transfer */
extern void *shmStart;
extern MCSharedTransition *shmTransitionTypeInfo;
extern void *shmTransitionData;
extern const size_t shmAllocationSize;

/* State */
extern MCDeferred<MCState> programState;

MC_CTOR void mc_init();
void mc_child_panic();
void mc_report_undefined_behavior(const char *);

#define MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(x, str)                \
  do {                                                              \
    if (!static_cast<bool>(x)) {                                    \
      mc_report_undefined_behavior(static_cast<const char *>(str)); \
    }                                                               \
  } while (0)

#define MC_REPORT_UNDEFINED_BEHAVIOR(str) \
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(false, str)

/* Scheduler state */
void mc_create_program_state();

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
tid_t thread_get_self();

/* Shared memory management */
void *mc_create_shared_memory_region();
void mc_initialize_shared_memory_region();

void mc_create_thread_sleep_points();
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
