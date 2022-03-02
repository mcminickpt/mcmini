#ifndef DPOR_CONCURRENT_SYSTEM_H
#define DPOR_CONCURRENT_SYSTEM_H

#include "decl.h"
#include "transition.h"
#include "thread.h"
#include "mutex.h"

STRUCT_DECL(concurrent_system)

extern thread_local tid_t tid_self;

void csystem_init(concurrent_system_ref);

tid_t csystem_register_thread(concurrent_system_ref);
tid_t csystem_register_main_thread(concurrent_system_ref);
thread_ref csystem_get_thread(concurrent_system_ref, tid_t);

mutex_ref csystem_map_mutex(concurrent_system_ref, pthread_mutex_t*);

void csystem_apply_transition(transition_ref);
void csystem_revert_transition(transition_ref);

#endif //DPOR_CONCURRENT_SYSTEM_H
