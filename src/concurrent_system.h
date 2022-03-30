#ifndef DPOR_CONCURRENT_SYSTEM_H
#define DPOR_CONCURRENT_SYSTEM_H

#include "decl.h"
#include "transition.h"
#include "thread.h"
#include "mutex.h"
#include "shm.h"
#include "state_stack_item.h"

STRUCT_DECL(concurrent_system)

// NOTE: tid_self is only valid in the threads
// of the child process
extern child_local thread_local tid_t tid_self;
extern concurrent_system csystem; /* Global concurrent system for the program */

void csystem_init(concurrent_system_ref);
void csystem_reset(concurrent_system_ref);

tid_t csystem_register_thread(concurrent_system_ref);
tid_t csystem_register_main_thread(concurrent_system_ref);

mutid_t csystem_register_mutex(concurrent_system_ref, pthread_mutex_t *);

/*
 * Visible Objects
 */
int csystem_get_thread_count(concurrent_system_ref);
bool csystem_is_registered_tid(concurrent_system_ref, tid_t);
thread_ref csystem_get_thread_with_tid(concurrent_system_ref, tid_t);
thread_ref csystem_get_thread_with_pthread(concurrent_system_ref, pthread_t);

int csystem_get_mutex_count(concurrent_system_ref);
mutex_ref csystem_get_mutex_with_mutid(concurrent_system_ref, mutid_t);
mutex_ref csystem_get_mutex_with_pthread(concurrent_system_ref, pthread_mutex_t*);

/*
 * State Stack
 */
int csystem_state_stack_count(concurrent_system_ref);
bool csystem_state_stack_is_empty(concurrent_system_ref);
static state_stack_item_ref csystem_grow_state_stack(concurrent_system_ref);
static state_stack_item_ref csystem_grow_state_stack_by_running_thread(concurrent_system_ref, thread_ref);
state_stack_item_ref csystem_shrink_state_stack(concurrent_system_ref);
state_stack_item_ref csystem_state_stack_top(concurrent_system_ref);
state_stack_item_ref csystem_state_stack_get_element(concurrent_system_ref ref, int i);
dynamic_transition_ref csystem_pop_first_enabled_transition_in_backtrack_set(concurrent_system_ref, state_stack_item_ref);

state_stack_item_ref csystem_pop_program_stacks_for_backtracking(concurrent_system_ref);

/*
 * Transition Stack
 */
int csystem_transition_stack_count(concurrent_system_ref);
bool csystem_transition_stack_is_empty(concurrent_system_ref);
bool csystem_is_in_deadlock(concurrent_system_ref ref);
transition_ref csystem_grow_transition_stack_by_running_thread(concurrent_system_ref, thread_ref);
transition_ref csystem_shrink_transition_stack(concurrent_system_ref);
transition_ref csystem_transition_stack_top(concurrent_system_ref);
transition_ref csystem_transition_stack_get_element(concurrent_system_ref, int);

void csystem_simulate_running_thread(concurrent_system_ref, shm_transition_ref, thread_ref);
dynamic_transition_ref csystem_get_transition_slot_for_tid(concurrent_system_ref, tid_t);
dynamic_transition_ref csystem_get_transition_slot_for_thread(concurrent_system_ref, csystem_local thread_ref);

dynamic_transition_ref csystem_get_first_enabled_transition(concurrent_system_ref);
int csystem_copy_enabled_transitions(concurrent_system_ref, transition_ref);
void csystem_copy_per_thread_transitions(concurrent_system_ref, transition_ref);

/*
 * Pretty printing
 */
void csystem_print_transition_stack(concurrent_system_ref);
void csystem_print_next_transitions_stack(concurrent_system_ref);

/*
 * Transitions
 */
transition_ref csystem_run(concurrent_system_ref, thread_ref);
void dpor_init_thread_start_transition(dynamic_transition_ref, thread_ref);
void dpor_init_thread_finish_transition(dynamic_transition_ref, thread_ref);

/**
 * Back tracking
 */
void csystem_dynamically_update_backtrack_sets(concurrent_system_ref);
bool csystem_p_q_could_race(concurrent_system_ref, int i, thread_ref q, thread_ref p);

/*
 * Happens before
 */
bool happens_before(concurrent_system_ref, int i, int j);
bool happens_before_thread(concurrent_system_ref, int i, thread_ref);

/*
 * Dynamic transitions
 */
dynamic_transition transition_convert_to_dynamic_transition_in_system(concurrent_system_ref, transition_ref);


#endif //DPOR_CONCURRENT_SYSTEM_H
