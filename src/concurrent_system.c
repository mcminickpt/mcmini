#include "concurrent_system.h"
#include "common.h"
#include "hashtable.h"
#include "fail.h"

thread_local tid_t tid_self = TID_INVALID;

struct concurrent_system
{
    tid_t tid_next; // <-- next available thread slot
    thread threads[MAX_TOTAL_THREADS_PER_SCHEDULE];

    hash_table_ref mutex_map;                               /* Maps pthread_mutex_t* to struct mutex* (pointer into the static array) */
    mutex locks[MAX_MUTEX_OBJECT_COUNT];                    /* Memory backing the mutexes in the map */

    int t_stack_top;                                        /* Points to the top of the transition stack */
    transition t_stack[MAX_VISIBLE_OPERATION_DEPTH];        /* *** TRANSITION STACK *** */
    transition t_next[MAX_TOTAL_THREADS_PER_SCHEDULE];      /* Storage for each transition for each thread */

    int s_stack_top;                                        /* Points to the top of the state stack */
    state_stack_item s_stack[MAX_VISIBLE_OPERATION_DEPTH];  /* *** STATE STACK *** */

    hash_table_ref transition_map;                          /* Maps struct thread* to transition_ref in t_next */
};

concurrent_system csystem; /* Global concurrent system for the program */

void
csystem_init(concurrent_system_ref ref)
{
    ref->tid_next = 0;
    ref->t_stack_top = -1;
    ref->s_stack_top = -1;
    ref->mutex_map = hash_table_create();
    ref->transition_map = hash_table_create();

    hash_table_set_hash_function(ref->transition_map, (hash_function)thread_hash);
    hash_table_set_hash_function(ref->mutex_map, (hash_function)mutex_hash);
}

void
csystem_reset(concurrent_system_ref ref)
{
    hash_table_destroy(ref->mutex_map);
    hash_table_destroy(ref->transition_map);
    csystem_init(ref);
}

tid_t
csystem_register_thread(concurrent_system_ref ref)
{
    tid_t self = ref->tid_next++;
    tid_self = self;
    thread_ref tself = &ref->threads[self];
    tself->arg = NULL;
    tself->start_routine = NULL;
    tself->owner = pthread_self();
    tself->tid = self;
    tself->is_alive = true;
    return self;
}

tid_t
csystem_register_main_thread(concurrent_system_ref ref)
{
    tid_t main = csystem_register_thread(ref);
    mc_assert(main == TID_MAIN_THREAD);
    return TID_MAIN_THREAD;
}

thread_ref
csystem_get_thread_with_tid(concurrent_system_ref ref, tid_t tid)
{
    if (tid == TID_INVALID) return NULL;
    return &ref->threads[tid];
}

thread_ref
csystem_get_thread_with_pthread(concurrent_system_ref ref, pthread_t *pthread)
{

}

mutex_ref
csystem_get_mutex_with_pthread(concurrent_system_ref ref, pthread_mutex_t *mutex)
{
    return (mutex_ref) hash_table_get(ref->mutex_map, (hash_t)mutex);
}

void
csystem_apply_transition(concurrent_system_ref ref, transition_ref transition)
{


}

void
csystem_revert_transition(concurrent_system_ref ref, transition_ref transition)
{

}

state_stack_item_ref
csystem_grow_state_stack(concurrent_system_ref ref)
{
    state_stack_item_ref s_top = &ref->s_stack[++ref->s_stack_top];
    s_top->backtrack_set = hash_set_create((hash_function) thread_hash);
    s_top->done_set = hash_set_create((hash_function) thread_hash);
    mc_assert(ref->s_stack_top <= MAX_VISIBLE_OPERATION_DEPTH);
    return s_top;
}

state_stack_item_ref
csystem_shrink_state_stack(concurrent_system_ref ref)
{
    mc_assert(ref->s_stack_top >= 0);
    state_stack_item_ref s_top = &ref->s_stack[ref->s_stack_top--];
    return s_top;
}

transition_ref
csystem_grow_transition_stack(concurrent_system_ref ref, thread_ref thread)
{
    transition_ref t_top = &ref->t_stack[++ref->t_stack_top];
    mc_assert(ref->t_stack_top <= MAX_VISIBLE_OPERATION_DEPTH);

    transition_ref thread_runs = csystem_get_transition_slot_for_thread(ref, thread);
    *t_top = *thread_runs;

    return t_top;
}

transition_ref
csystem_shrink_transition_stack(concurrent_system_ref ref)
{
    mc_assert(ref->t_stack_top >= 0);
    transition_ref t_top = &ref->t_stack[ref->t_stack_top--];

    transition_ref was_next = csystem_get_transition_slot_for_thread(ref, t_top->thread);
    *was_next = *t_top; /* t_top represents what executed BEFORE to reach the current next transition */

    return t_top;
}

transition_ref
csystem_get_transition_slot_for_thread(concurrent_system_ref ref, csystem_local thread_ref thread)
{
    tid_t tid = thread->tid;
    return &ref->t_next[tid];
}

transition_ref
csystem_get_first_enabled_transition(concurrent_system_ref ref)
{
    for (int i = 0; i < MAX_TOTAL_THREADS_PER_SCHEDULE; i++) {
        transition_ref t_next_i = &ref->t_next[i];
        if (transition_enabled(t_next_i)) return t_next_i;
    }
    return NULL;
}