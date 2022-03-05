#include "concurrent_system.h"
#include "common.h"
#include "hashtable.h"
#include "fail.h"

thread_local tid_t tid_self = TID_INVALID;

struct concurrent_system
{
    tid_t tid_next;                                         /* Next available thread slot */
    thread threads[MAX_TOTAL_THREADS_PER_SCHEDULE];         /* Thread identities for all threads in the system */

    hash_table_ref mutex_map;                               /* Maps pthread_mutex_t* to struct mutex* (pointer into the static array) */
    mutex locks[MAX_MUTEX_OBJECT_COUNT];                    /* Memory backing the mutexes in the map */

    int t_stack_top;                                        /* Points to the top of the transition stack */
    transition t_stack[MAX_VISIBLE_OPERATION_DEPTH];        /* *** TRANSITION STACK *** */
    transition t_next[MAX_TOTAL_THREADS_PER_SCHEDULE];      /* Storage for each next transition (next(s, p))= for each thread */
    hash_table_ref transition_map;                          /* Maps `struct thread*` to `transition_ref` in `t_next` */

    int s_stack_top;                                        /* Points to the top of the state stack */
    state_stack_item s_stack[MAX_VISIBLE_OPERATION_DEPTH];  /* *** STATE STACK *** */

    // *** BACKTRACKING ***
    bool is_backtracking;                                   /* Whether or not the system is performing backtrack analysis*/
    int detached_t_top;                                     /* Points to the top of the transition stack in the current backtrack */
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

inline thread_ref
csystem_get_thread_with_pthread(concurrent_system_ref ref, pthread_t *pthread)
{

}

inline mutex_ref
csystem_get_mutex_with_pthread(concurrent_system_ref ref, pthread_mutex_t *mutex)
{
    return (mutex_ref) hash_table_get(ref->mutex_map, (hash_t)mutex);
}

static void
csystem_apply_transition(concurrent_system_ref ref, transition_ref transition)
{
    // Copy the contents of the transition into the the top of the transition stack
    ref->t_stack[ref->t_stack_top] = *transition;

    // TODO: Do the application here
}

static void
csystem_revert_transition(concurrent_system_ref ref, transition_ref transition)
{

    transition_ref was_next = csystem_get_transition_slot_for_thread(ref, transition->thread);
    *was_next = *transition; /* t_top represents what executed BEFORE to reach the current next transition */

    // TODO: Do the revert here
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

inline state_stack_item_ref
csystem_shrink_state_stack(concurrent_system_ref ref)
{
    mc_assert(ref->s_stack_top >= 0);
    state_stack_item_ref s_top = &ref->s_stack[ref->s_stack_top--];
    return s_top;
}

inline transition_ref
csystem_grow_transition_stack(concurrent_system_ref ref, thread_ref thread)
{
    transition_ref t_top = &ref->t_stack[++ref->t_stack_top];
    mc_assert(ref->t_stack_top <= MAX_VISIBLE_OPERATION_DEPTH);

    transition_ref thread_runs = csystem_get_transition_slot_for_thread(ref, thread);
    csystem_apply_transition(ref, thread_runs);
    return t_top;
}

inline transition_ref
csystem_shrink_transition_stack(concurrent_system_ref ref)
{
    mc_assert(ref->t_stack_top >= 0);
    transition_ref t_top = &ref->t_stack[ref->t_stack_top--];
    csystem_revert_transition(ref, t_top);

    return t_top;
}

inline transition_ref
csystem_get_transition_slot_for_thread(concurrent_system_ref ref, csystem_local thread_ref thread)
{
    return csystem_get_transition_slot_for_tid(ref, thread->tid);
}

inline transition_ref
csystem_get_transition_slot_for_tid(concurrent_system_ref ref, csystem_local tid_t tid)
{
    mc_assert(tid != TID_INVALID);
    return &ref->t_next[tid];
}

int
csystem_copy_enabled_transitions(concurrent_system_ref ref, transition_ref tref_array)
{
    int thread_count = csystem_get_thread_count(ref);
    int enabled_thread_count = 0;

    for (tid_t tid = 0; tid < thread_count; tid++) {
        transition_ref tid_transition = csystem_get_transition_slot_for_tid(ref, tid);

        if (transition_enabled(tid_transition)) {
            tref_array[enabled_thread_count++] = *tid_transition;
        }
    }
    return enabled_thread_count;
}

inline transition_ref
csystem_transition_stack_top(concurrent_system_ref ref)
{
    mc_assert(ref->t_stack_top >= 0 && ref->t_stack_top < MAX_VISIBLE_OPERATION_DEPTH);
    return &ref->t_stack[ref->t_stack_top];
}

inline state_stack_item_ref
csystem_state_stack_top(concurrent_system_ref ref)
{
    mc_assert(ref->s_stack_top >= 0 && ref->s_stack_top < MAX_VISIBLE_OPERATION_DEPTH);
    return &ref->s_stack[ref->s_stack_top];
}

void
csystem_copy_per_thread_transitions(concurrent_system_ref ref, transition_ref tref_array)
{
    int thread_count = csystem_get_thread_count(ref);
    int enabled_thread_index = 0;

    for (tid_t tid = 0; tid < thread_count; tid++) {
        transition_ref tid_transition = csystem_get_transition_slot_for_tid(ref, tid);
        tref_array[enabled_thread_index++] = *tid_transition;
    }
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

inline int
csystem_get_thread_count(concurrent_system_ref ref)
{
    return (int)ref->tid_next;
}

inline int
csystem_state_stack_count(concurrent_system_ref ref)
{
    return ref->s_stack_top + 1;
}

inline int
csystem_transition_stack_count(concurrent_system_ref ref)
{
    return ref->t_stack_top + 1;
}

inline bool
csystem_state_stack_is_empty(concurrent_system_ref ref)
{
    return ref->s_stack_top < 0;
}

inline bool
csystem_transition_stack_is_empty(concurrent_system_ref ref)
{
    return ref->t_stack_top < 0;
}

void
csystem_start_backtrack(concurrent_system_ref ref)
{
    mc_assert(!ref->is_backtracking);
    ref->is_backtracking = true;
}

void
csystem_end_backtrack(concurrent_system_ref ref)
{
    mc_assert(ref->is_backtracking);
    ref->is_backtracking = false;

    // TODO: Restore state before backtracking
}

bool
happens_before(concurrent_system_ref ref, int i, int j)
{
    transition_ref t_i = &ref->t_stack[i];
    transition_ref t_j = &ref->t_stack[j];
    return i <= j && transitions_dependent(t_i, t_j);
}

bool
happens_before_thread(concurrent_system_ref ref, int i, thread_ref p)
{
    for (int k = i; k <= ref->t_stack_top; k++) {
        transition_ref S_k = &ref->t_stack[k];
        if (happens_before(ref, i, k) && threads_equal(p, S_k->thread))
            return true;
    }
    return false;
}

bool
csystem_p_q_could_race(concurrent_system_ref ref, int i, thread_ref q, thread_ref p)
{
    mc_assert(ref->is_backtracking);
    for (int j = i + 1; j <= ref->t_stack_top; j++) {
        transition_ref S_j = &ref->t_stack[j];
        if (threads_equal(p, S_j->thread) && happens_before_thread(ref, j, p))
            return true;
    }
    return false;
}