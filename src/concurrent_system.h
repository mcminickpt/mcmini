#ifndef DPOR_CONCURRENT_SYSTEM_H
#define DPOR_CONCURRENT_SYSTEM_H

#include "decl.h"
#include "transition.h"
#include "thread.h"
#include "mutex.h"
#include "state_stack_item.h"

STRUCT_DECL(concurrent_system)

extern thread_local tid_t tid_self;

void csystem_init(concurrent_system_ref);
void csystem_reset(concurrent_system_ref);

tid_t csystem_register_thread(concurrent_system_ref);
tid_t csystem_register_main_thread(concurrent_system_ref);

/*
 * Visible Objects
 */
int csystem_get_thread_count(concurrent_system_ref);
thread_ref csystem_get_thread_with_tid(concurrent_system_ref, tid_t);
thread_ref csystem_get_thread_with_pthread(concurrent_system_ref, pthread_t*);
mutex_ref csystem_get_mutex_with_pthread(concurrent_system_ref, pthread_mutex_t*);

/*
 * State Stack
 */
int csystem_state_stack_count(concurrent_system_ref);
bool csystem_state_stack_is_empty(concurrent_system_ref);
state_stack_item_ref csystem_grow_state_stack(concurrent_system_ref);
state_stack_item_ref csystem_shrink_state_stack(concurrent_system_ref);

/*
 * Transition Stack
 */
int csystem_transition_stack_count(concurrent_system_ref);
bool csystem_transition_stack_is_empty(concurrent_system_ref);
transition_ref csystem_grow_transition_stack(concurrent_system_ref, thread_ref);
transition_ref csystem_shrink_transition_stack(concurrent_system_ref);

transition_ref csystem_get_transition_slot_for_tid(concurrent_system_ref, tid_t);
transition_ref csystem_get_transition_slot_for_thread(concurrent_system_ref, csystem_local thread_ref);

transition_ref csystem_get_first_enabled_transition(concurrent_system_ref);
int csystem_copy_enabled_transitions(concurrent_system_ref, transition_ref);


#endif //DPOR_CONCURRENT_SYSTEM_H
