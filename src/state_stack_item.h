#ifndef DPOR_STATE_STACK_ITEM_H
#define DPOR_STATE_STACK_ITEM_H

#include "thread.h"
#include "shared_state.h"

struct state_stack_item {
    struct shared_state state;
    thread_array backtrack_set;
    thread_array done_set;
    // Other stuff
};

// Collection of state_stack_items
typedef struct state_stack state_stack;

#endif //DPOR_STATE_STACK_ITEM_H
