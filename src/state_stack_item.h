#ifndef DPOR_STATE_STACK_ITEM_H
#define DPOR_STATE_STACK_ITEM_H

#include "thread.h"
#include "array.h"
#include "shared_state.h"

STRUCT_DECL(state_stack_item);
struct state_stack_item {
    shared_state_ref state;
    thread_array_ref backtrack_set; // possibly a hash_set?
    thread_array_ref done_set; // possibly a hash_set
};
typedef array_ref state_stack_ref;
typedef array_ref transtion_stack_ref;
MEMORY_API_DECL(state_stack_item);


transition_ref state_stack_item_get_first_enabled_backtrack_transition(state_stack_item_refc);
void state_stack_item_remove_backtrack_thread(state_stack_item_ref, thread_ref);
void state_stack_item_mark_thread_as_searched(state_stack_item_ref, thread_ref);

#endif //DPOR_STATE_STACK_ITEM_H
