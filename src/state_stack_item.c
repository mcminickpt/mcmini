#include "state_stack_item.h"

MEMORY_ALLOC_DEF_DECL(state_stack_item)

state_stack_item_ref
state_stack_item_create(void)
{
    state_stack_item_ref ref = state_stack_item_alloc();
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
state_stack_item_copy(state_stack_item_refc ref)
{

}

void
state_stack_item_destroy(state_stack_item_ref ref)
{

}
thread_ref
proc(transition_refc ref) {
    return ref != NULL ? ref->thread : NULL;
}

transition_ref
state_stack_item_get_first_enabled_backtrack_transition(state_stack_item_refc ss_item)
{
    if (!ss_item) return NULL;
    return NULL;
}

void
state_stack_item_remove_backtrack_thread(state_stack_item_ref ss_item, thread_ref thread)
{
    if (!ss_item || !thread) return;

    uint32_t count = array_count(ss_item->backtrack_set);
    for (uint32_t i = 0; i < count; i++) {
        thread_ref threadi = array_get(ss_item->backtrack_set, i);
        if (threads_equal(threadi, thread)) {
            array_remove(ss_item->backtrack_set, i);
            return;
        }
    }
}

void
state_stack_item_mark_thread_as_searched(state_stack_item_ref ss_item, thread_ref thread)
{
    if (!ss_item || !thread) return;

    // TODO: This technically might repeat a thread
    // prevent duplicates with a hash set
    array_append(ss_item->done_set, thread);
}
