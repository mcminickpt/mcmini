#ifndef DPOR_DPOR_H
#define DPOR_DPOR_H

#include "othread.h"
#include "state_stack_item.h"
#include "transition.h"

#define MAX_TOTAL_THREADS_PER_SCHEDULE 10u
#define MAX_VISIBLE_OPERATION_DEPTH 1000u

typedef uint64_t tid_t;
#define TID_MAIN_THREAD 0ul;
#define TID_INVALID -1ul; // ULONG_MAX

extern thread_local tid_t thread_self;
extern tid_t tid_next;
extern state_stack_ref s_stack;
extern transition_array_ref t_stack;
extern thread *threads; /* MAX_TOTAL_THREADS_PER_SCHEDULE size */
extern coop_lock *queue;
extern shm_transition_ref shm_child_result;

/**
 * Operations
 */
void dpor_init(void);
tid_t dpor_register_thread(void);
tid_t dpor_register_main_thread(void);
void thread_await_dpor_scheduler(void);

thread_ref thread_get_self(void);

/* DPOR Functions */
void dynamically_update_backtrack_sets(state_stack_item_ref);

#endif //DPOR_DPOR_H
