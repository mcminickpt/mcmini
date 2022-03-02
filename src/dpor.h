#ifndef DPOR_DPOR_H
#define DPOR_DPOR_H

#include "common.h"
#include "mc_shared_cv.h"
#include "state_stack_item.h"
#include "shm.h"
#include "concurrent_system.h"
#include "hashtable.h"
#include "transition.h"

extern state_stack_ref s_stack;
extern transition_stack_ref t_stack;
extern concurrent_system csystem;

extern mc_shared_cv (*queue)[MAX_TOTAL_THREADS_PER_SCHEDULE];
extern shm_transition_ref shm_child_result;

/**
 * Operations
 */
void dpor_init(void);

static bool dpor_parent_scheduler_loop(uint32_t max_depth);
static bool dpor_spawn_child(void);
static bool dpor_spawn_child_following_transition_stack(void);
static void dpor_child_exit(void);
static void dpor_child_kill(void);
static void dpor_run(tid_t tid);
static void *dpor_create_shared_memory_region(void);
static void dpor_initialize_shared_memory_region(void);
static void dpor_reset_cv_locks(void);

void thread_await_dpor_scheduler(void);
void thread_await_dpor_scheduler_initialization(void);
void dynamically_update_backtrack_sets(state_stack_item_ref);

#endif //DPOR_DPOR_H
