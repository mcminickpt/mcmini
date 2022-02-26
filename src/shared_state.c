#include "shared_state.h"
#include "common.h"

MEMORY_ALLOC_DEF_DECL(shared_state)

shared_state_ref
shared_state_create(void) {
    shared_state_ref ref = shared_state_alloc();
    if (!ref) goto f1;

    if ((ref->threads = array_create()) == NULL)
        goto f2;

    if ((ref->mutexes = array_create()) == NULL)
        goto f3;

    if ((ref->transitions = array_create()) == NULL)
        goto f4;
    return ref;

f4:
    array_destroy(ref->mutexes, NULL);
f3:
    array_destroy(ref->threads, NULL);
f2:
    free(ref);
f1:
    return NULL;
}

shared_state_ref
shared_state_copy(shared_state_refc other) {
    if (!other) return NULL;
    shared_state_ref cpy = shared_state_alloc();
    if (!cpy) return NULL;
    cpy->mutexes = array_deep_cpy(other->mutexes, mutex_copy);
    cpy->transitions = array_deep_cpy(other->transitions, transition_copy);
    cpy->threads = array_deep_cpy(other->threads, thread_copy);
    return cpy;
}

void
shared_state_destroy(shared_state_ref ref) {
    if (!ref) return;
    array_destroy(ref->transitions, (free_function)transition_destroy);
    array_destroy(ref->mutexes, (free_function)mutex_destroy);
    array_destroy(ref->threads, (free_function)thread_destroy);
    free(ref);
}

transition_ref
shared_state_get_first_enabled_transition(shared_state_refc ref) {
    if (!ref) return NULL;
    const uint32_t count = array_count(ref->transitions);
    for (uint32_t i = 0; i < count; i++) {
        transition_ref t = array_get(ref->transitions, i);
        if (transition_enabled(t)) return t;
    }
    return NULL;
}
transition_ref
shared_state_get_first_enabled_transition_by_thread(shared_state_refc ref, thread_ref thread)
{
    if (!ref || !thread) return NULL;
    const uint32_t count = array_count(ref->transitions);
    for (uint32_t i = 0; i < count; i++) {
        transition_ref t = array_get(ref->transitions, i);
        if (transition_enabled(t) && threads_equal(t->thread, thread)) return t;
    }
    return NULL;
}

transition_array_ref
shared_state_create_enabled_transitions(shared_state_refc ref)
{
    if (!ref) return NULL;
    array_ref array = array_create();
    uint32_t count = array_count(ref->transitions);
    for (uint32_t i = 0; i < count; i++) {
        transition_ref get = array_get(array, i);
        if (transition_enabled(get))
            array_append(array, transition_copy(get));
    }
    return array;
}

bool
shared_state_has_mutex(shared_state_ref state, mutex_refc mutex)
{
    if (!state || !mutex) return false;

    uint32_t count = array_count(state->mutexes);
    for (uint32_t i = 0; i < count; i++) {
        mutex_ref mut = array_get(state->mutexes, i);
        if (mutexes_equal(mut, mutex)) return true;
    }
    return false;
}

shared_state_ref
next(shared_state_ref ss_ref, transition_ref t_executed, transition_ref t_next)
{
    if (!ss_ref || !t_executed || !t_next) return NULL;

    shared_state_ref cpy = shared_state_copy(ss_ref);
    switch (t_executed->operation->type) {
        case MUTEX:;
            mutex_operation_ref mutop = t_executed->operation->mutex_operation;

            // Search for the mutexes to see if we already have this one
            if (shared_state_has_mutex(cpy, mutop->mutex))
                break;

            // TODO: Write this in a separate function
            switch (mutop->type) {
                case MUTEX_INIT:;

                mutex_ref mut = mutop->mutex;
                mut->state = MUTEX_UNLOCKED;
                mut->owner = NULL;

                default:
                    break;
            }

            break;

        case THREAD_LIFECYCLE:;
            break;
        default:
            return NULL;
    }

    shared_state_update_next_transition_for_thread(cpy, t_executed->thread, t_next);
    return cpy;
}

void
shared_state_update_next_transition_for_thread(shared_state_ref state, thread_ref thread, transition_ref transition)
{
    if (!state || !thread || !transition) return;

    uint32_t transitions = array_count(state->transitions);
    for (uint32_t i = 0; i < transitions; i++) {
        transition_ref ti = array_get(state->transitions, i);

        if (threads_equal(ti->thread, thread)) {
            array_set(state->transitions, i, (const void**)&transition);
            return;
        }
    }
}