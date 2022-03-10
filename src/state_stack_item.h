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

#endif //DPOR_STATE_STACK_ITEM_H
