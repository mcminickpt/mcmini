#ifndef DPOR_DPOR_H
#define DPOR_DPOR_H

#include "common.h"
#include "mc_shared_cv.h"
#include "state_stack_item.h"
#include "shm.h"
#include "concurrent_system.h"
#include "hashtable.h"
#include "transition.h"

extern mc_shared_cv (*queue)[MAX_TOTAL_THREADS_PER_SCHEDULE];
extern shm_transition_ref shm_child_result;

/**
 * Operations
 */
void dpor_init(void);
static bool dpor_scheduler_main();

static void dpor_print_shared_cvs(void);
static tid_t dpor_register_thread(void);
static tid_t dpor_register_main_thread(void);

static bool dpor_begin_target_program_at_main(void);
static bool dpor_spawn_child(void);
static bool dpor_spawn_child_following_transition_stack(void);

static void dpor_child_kill(void);
static void dpor_run_thread_to_next_visible_operation(tid_t tid);
static void *dpor_create_shared_memory_region(void);
static void dpor_initialize_shared_memory_region(void);
static void dpor_reset_cv_locks(void);

void thread_await_dpor_scheduler(void);
void thread_await_dpor_scheduler_for_thread_start_transition(void);
void thread_awake_dpor_scheduler_for_thread_finish_transition(void);

#endif //DPOR_DPOR_H
