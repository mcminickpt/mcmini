#ifndef DPOR_STATE_STACK_ITEM_H
#define DPOR_STATE_STACK_ITEM_H

#include "array.h"
#include "hashset.h"
#include "thread.h"
#include "shared_state.h"

STRUCT_DECL(state_stack_item);
struct state_stack_item {
    hash_set_ref backtrack_set;     /* Set of thread_ref that need to be scheduled */
    hash_set_ref done_set;          /* Set of thread_ref that have been scheduled and don't need to be checked */
};
typedef array_ref state_stack_ref;
typedef array_ref transtion_stack_ref;


transition_ref state_stack_item_get_first_enabled_backtrack_transition(state_stack_item_refc);
void state_stack_item_remove_backtrack_thread(state_stack_item_ref, thread_ref);
void state_stack_item_mark_thread_as_searched(state_stack_item_ref, thread_ref);

#endif //DPOR_STATE_STACK_ITEM_H
