#ifndef DPOR_DPOR_H
#define DPOR_DPOR_H

#include "mc_shared_cv.h"
#include "state_stack_item.h"
#include "shm.h"
#include "transition.h"

#define MAX_TOTAL_THREADS_PER_SCHEDULE 10u
#define MAX_VISIBLE_OPERATION_DEPTH 5u

typedef uint64_t tid_t;
#define TID_MAIN_THREAD (0ul)
#define TID_INVALID (-1ul) // ULONG_MAX

extern thread_local tid_t thread_self;
extern tid_t tid_next;
extern state_stack_ref s_stack;
extern transition_array_ref t_stack;
extern thread threads[MAX_TOTAL_THREADS_PER_SCHEDULE];
extern mc_shared_cv (*queue)[MAX_TOTAL_THREADS_PER_SCHEDULE];
extern shm_transition_ref shm_child_result;

/**
 * Operations
 */
void dpor_init(void);
tid_t dpor_register_thread(void);
tid_t dpor_register_main_thread(void);

static void dpor_sigusr2(int);
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
