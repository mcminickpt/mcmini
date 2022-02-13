#include "state_stack_item.h"

state_stack_item_ref
state_stack_item_create(void)
{
    state_stack_item_ref ref = malloc(sizeof(*ref));
    if (!ref) return NULL;

    if ((ref->state = shared_state_create()) == NULL)
        goto f1;

    if ((ref->backtrack_set = array_create()) == NULL)
        goto f2;

    if ((ref->done_set = array_create()) == NULL)
        goto f3;
    return ref;
f3:
    array_destroy(ref->backtrack_set, NULL);
f2:
    shared_state_destroy(ref->state);
f1:
    free(ref);
    return NULL;
}

state_stack_item_ref
state_stack_item_copy(state_stack_item_refc ref) {

}

void
state_stack_item_destroy(state_stack_item_ref ref)
{

}