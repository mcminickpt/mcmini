#ifndef DPOR_DPOR_H
#define DPOR_DPOR_H

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "othread.h"
#include "state_stack_item.h"
#include "transition.h"

STRUCT_DECL(dpor_context);
struct dpor_context {
    state_stack_ref state_stack;
    transtion_stack_ref transition_stack;

    othread_ref main_thread;
    othread_array_ref thread_table;
};
MEMORY_API_DECL(dpor_context);

dpor_context_ref dpor_shared;
static sem_t dpor_init_sem;
static sem_t main_thread_init_sem;

/**
 * Operations
 */
void dpor_init(void);
void dpor_run(dpor_context_ref, thread_ref);
void dpor_register_thread(dpor_context_ref, pthread_t);
void dpor_register_main_thread(dpor_context_ref, pthread_t);

void thread_await_dpor_scheduler(dpor_context_ref);
void dynamically_update_backtrack_sets(dpor_context_ref, state_stack_item_ref);


#endif //DPOR_DPOR_H
