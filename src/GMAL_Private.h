#ifndef GMAL_GMAL_PRIVATE_H
#define GMAL_GMAL_PRIVATE_H

#include "GMALShared.h"
#include "GMALSharedTransition.h"
#include "GMALState.h"

extern "C" {
    #include "mc_shared_cv.h"
}

/* Synchronization primitives */
extern GMAL_THREAD_LOCAL tid_t tid_self;
extern pid_t cpid;
extern mc_shared_cv (*threadQueue)[MAX_TOTAL_THREADS_IN_PROGRAM];
extern sem_t gmal_pthread_create_binary_sem;

/* Data transfer */
extern void *shmStart;
extern GMALSharedTransition *shmTransitionData;
extern const size_t shmAllocationSize;

/* State */
extern GMALState programState;

/* Scheduler state */
void gmal_create_program_state();

/* Scheduler control */
GMAL_PROGRAM_TYPE gmal_scheduler_main();
GMAL_PROGRAM_TYPE gmal_readvance_main();
void gmal_create_initial_scheduler_state();
void gmal_exit();

/* Registering and accessing threads */
tid_t gmal_register_thread();
tid_t gmal_register_main_thread();
tid_t thread_get_self();

/* Shared memory management */
void* gmal_create_shared_memory_region();
void gmal_initialize_shared_memory_region();

void gmal_create_thread_sleep_points();
void gmal_reset_thread_sleep_points();


/* Source program management */
GMAL_PROGRAM_TYPE gmal_spawn_child();
GMAL_PROGRAM_TYPE gmal_spawn_child_following_transition_stack();
GMAL_PROGRAM_TYPE gmal_begin_target_program_at_main();
void gmal_run_thread_to_next_visible_operation(tid_t);
void gmal_child_kill();

/* Source program thread control */
void thread_await_gmal_scheduler();
void thread_await_gmal_scheduler_for_thread_start_transition();
void thread_awake_gmal_scheduler_for_thread_finish_transition();

#endif //GMAL_GMAL_PRIVATE_H
