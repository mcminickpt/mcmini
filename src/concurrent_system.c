#include <stdio.h>
#include "concurrent_system.h"
#include "common.h"
#include "hashtable.h"
#include "fail.h"

thread_local tid_t tid_self = TID_INVALID;

struct concurrent_system
{
    tid_t tid_next;                                                 /* Next available thread slot */
    thread threads[MAX_TOTAL_THREADS_PER_SCHEDULE];                 /* Thread identities for all threads in the system */

    mutid_t mut_next;                                               /* Next available mutex slot */
    hash_table_ref mutex_map;                                       /* Maps pthread_mutex_t* to struct mutex* (pointer into the static array) */
    mutex locks[MAX_MUTEX_OBJECT_COUNT];                            /* Memory backing the mutexes in the map */

    int t_stack_top;                                                /* Points to the top of the transition stack */
    transition t_stack[MAX_VISIBLE_OPERATION_DEPTH];                /* *** TRANSITION STACK *** */
    dynamic_transition t_next[MAX_TOTAL_THREADS_PER_SCHEDULE];      /* Storage for each next transition (next(s, p)) = for each thread */
    hash_table_ref transition_map;                                  /* Maps `struct thread*` to `transition_ref` in `t_next` */

    int s_stack_top;                                                /* Points to the top of the state stack */
    state_stack_item s_stack[MAX_VISIBLE_OPERATION_DEPTH];          /* *** STATE STACK *** */

    // *** BACKTRACKING ***
    bool is_backtracking;                                           /* Whether or not the system is performing backtrack analysis*/
    int detached_t_top;                                             /* Points to the top of the transition stack in the current backtrack */
    int detached_s_top;                                             /* Points to the top of the state stack in the current backtrack */
};

concurrent_system csystem; /* Global concurrent system for the program */

void
csystem_init(concurrent_system_ref ref)
{
    ref->tid_next = 0;
    ref->t_stack_top = -1;
    ref->s_stack_top = -1;
    ref->mut_next = 0;
    ref->is_backtracking = false;
    ref->detached_t_top = -1;
    ref->detached_s_top = -1;
    ref->mutex_map = hash_table_create((hash_function) pthread_mutex_hash,
                                       (hash_equality_function) dpor_pthread_mutexes_equal);
    ref->transition_map = hash_table_create((hash_function)thread_hash, (hash_equality_function) threads_equal);

    // Push the initial first state (the starting state) onto the state stack explicitly
    csystem_grow_state_stack(ref);
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
    tself->state = THREAD_ALIVE;
    return self;
}

tid_t
csystem_register_main_thread(concurrent_system_ref ref)
{
    tid_t main = csystem_register_thread(ref);
    mc_assert(main == TID_MAIN_THREAD);
    return TID_MAIN_THREAD;
}

mutid_t
csystem_register_mutex(concurrent_system_ref ref, pthread_mutex_t *mut)
{
    mutid_t new = ref->mut_next++;
    mutex_ref mnew = &ref->locks[new];
    mnew->state = MUTEX_UNKNOWN;
    mnew->owner = NULL;
    mnew->mutex = mut;
    mnew->mutid = new;
    hash_table_set(ref->mutex_map, mut, mnew);
    return new;
}

mutex_ref
csystem_get_mutex_with_mutid(concurrent_system_ref ref, mutid_t mutid)
{
    mc_assert(mutid >= 0 && mutid < MAX_MUTEX_OBJECT_COUNT);
    if (mutid == MUTID_INVALID) return NULL;
    return &ref->locks[mutid];
}

inline thread_ref
csystem_get_thread_with_tid(concurrent_system_ref ref, tid_t tid)
{
    mc_assert(tid != TID_INVALID);
    mc_assert(tid >= 0 && tid < MAX_TOTAL_THREADS_PER_SCHEDULE);
    if (tid == TID_INVALID) return NULL;
    return &ref->threads[tid];
}

thread_ref
csystem_get_thread_with_pthread(concurrent_system_ref ref, pthread_t pthread_identity)
{
    const int t_stack_top = csystem_transition_stack_count(ref);
    for (int i = 0; i < ref->tid_next; i++) {
        thread_ref t = &ref->threads[i];
        if (pthread_equal(t->owner, pthread_identity)) {
            return t;
        }
    }
    return NULL;
}

inline bool
csystem_is_registered_tid(concurrent_system_ref ref, tid_t tid)
{
    if (tid == TID_INVALID) return false;
    return tid < ref->tid_next;
}

inline mutex_ref
csystem_get_mutex_with_pthread(concurrent_system_ref ref, pthread_mutex_t *mutex)
{
    return (mutex_ref) hash_table_get(ref->mutex_map, mutex);
}

static inline int
csystem_get_top_of_transition_stack_based_on_context(concurrent_system_ref ref)
{
    if (ref->is_backtracking) {
        mc_assert(ref->detached_t_top >= 0);
        return ref->detached_t_top;
    } else {
        return ref->t_stack_top;
    }
}

static inline int
csystem_get_top_of_state_stack_based_on_context(concurrent_system_ref ref)
{
    if (ref->is_backtracking) {
        mc_assert(ref->detached_s_top >= 0);
        return ref->detached_s_top;
    } else {
        return ref->s_stack_top;
    }
}

static void
csystem_update_system_with_mutex_operation(concurrent_system_ref ref, dynamic_transition_ref next_transition_slot, shm_mutex_operation_ref shmmop)
{
    // TODO: Report any undefined behavior with the transition we derive
    next_transition_slot->operation.type = MUTEX;
    next_transition_slot->operation.mutex_operation.type = shmmop->type;

    // In the special case of mutex creation, we must register
    // the mutex on the side of the scheduler
    mutex_ref shadow_mutex = hash_table_get(ref->mutex_map, shmmop->mutex);
    const bool mutex_didnt_exist = shadow_mutex == NULL;
    if (mutex_didnt_exist) {
        mutid_t mutid = csystem_register_mutex(ref, shmmop->mutex);
        shadow_mutex = csystem_get_mutex_with_mutid(ref, mutid);
        next_transition_slot->operation.mutex_operation.mutex = shadow_mutex;
    } else {
        next_transition_slot->operation.mutex_operation.mutex = shadow_mutex;
    }

    // Undefined mutex behavior e.g.
    if (mutex_didnt_exist) {
        switch (shmmop->type) {
            case MUTEX_INIT:
                break;
            default:
                mc_report_undefined_behavior();
        }
    } else {
        // Init twice
    }
}

static void
csystem_update_system_with_thread_operation(concurrent_system_ref ref, dynamic_transition_ref next_transition_slot, shm_thread_operation_ref shmtop)
{
    // TODO: Report any undefined behavior with the transition we derive
    next_transition_slot->operation.type = THREAD_LIFECYCLE;
    next_transition_slot->operation.thread_operation.type = shmtop->type;

    switch (shmtop->type) {
        case THREAD_CREATE:;
            // In the special case of thread creation, we must
            // register the thread on the side of the scheduler
            tid_t new_thread_tid = csystem_register_thread(ref);
            thread_ref new_thread = csystem_get_thread_with_tid(ref, new_thread_tid);
            next_transition_slot->operation.thread_operation.thread = new_thread;

            // Also insert a new operation for the new thread
            dynamic_transition_ref t_slot_new_thread = csystem_get_transition_slot_for_thread(ref, new_thread);
            dpor_init_thread_start_transition(t_slot_new_thread, new_thread);
            break;
        case THREAD_JOIN:
            // Complete implementation
            mc_unimplemented();
//            thread_ref referenced_thread = csystem_get_thread_with_pthread(ref, shmtop->target);
//            next_transition_slot->operation.thread_operation.thread = referenced_thread;
            break;
        case THREAD_START:
            break;
        default:
            next_transition_slot->operation.thread_operation.thread = csystem_get_thread_with_tid(ref, shmtop->tid);
            break;
    }
}

static void
csystem_update_next_transition_for_thread_with_next_source_program_visible_operation(concurrent_system_ref ref, shm_transition_ref shm_ref, thread_ref thread)
{
    tid_t shmtid = shm_ref->thread.tid;
    thread_ref shmthread = csystem_get_thread_with_tid(ref, shmtid);
    dynamic_transition_ref tref = csystem_get_transition_slot_for_thread(ref, thread);
    tref->thread = shmthread;

    visible_operation_type type = shm_ref->operation.type;
    switch (type) {
        case MUTEX:;
            shm_mutex_operation_ref shmmop = &shm_ref->operation.mutex_operation;
            csystem_update_system_with_mutex_operation(ref, tref, shmmop);
            break;
        case THREAD_LIFECYCLE:;
            shm_thread_operation_ref shmtop = &shm_ref->operation.thread_operation;
            csystem_update_system_with_thread_operation(ref, tref, shmtop);
            break;
        default:
            mc_unimplemented();
            return;
    }
}

void
csystem_simulate_running_thread(concurrent_system_ref ref, shm_transition_ref shm_ref, thread_ref thread)
{
    csystem_grow_state_stack_by_running_thread(ref, thread);
    csystem_grow_transition_stack_by_running_thread(ref, thread);
    csystem_update_next_transition_for_thread_with_next_source_program_visible_operation(ref, shm_ref, thread);
}

static void
csystem_virtually_apply_mutex_operation(concurrent_system_ref ref, mutex_operation_ref mutop, bool execute_init_op)
{
    pthread_mutex_t *mutex = mutop->mutex.mutex;
    mutex_ref shadow = NULL;

    switch (mutop->type) {
        case MUTEX_INIT:;
            if (!execute_init_op) return;
            mutid_t mutid = csystem_register_mutex(ref, mutex);
            shadow = csystem_get_mutex_with_mutid(ref, mutid);
            shadow->mutid = mutid;
            shadow->state = MUTEX_UNLOCKED;
            shadow->owner = NULL;
            shadow->mutex = mutex;
            hash_table_set(ref->mutex_map, mutex, shadow);
            break;
        case MUTEX_LOCK:;
            shadow = hash_table_get(ref->mutex_map, mutex);
            shadow->state = MUTEX_LOCKED;
            break;
        case MUTEX_UNLOCK:
            shadow = hash_table_get(ref->mutex_map, mutex);
            shadow->state = MUTEX_UNLOCKED;
            break;
        default:
            mc_unimplemented();
    }
}

static void
csystem_virtually_apply_thread_operation(concurrent_system_ref ref, thread_operation_ref top, bool execute_init_op)
{
    // TODO: Report undefined behavior here
    switch (top->type) {
        case THREAD_CREATE:;
            if (!execute_init_op) return;
            // "Create" a new thread
            ref->tid_next++;
            break;
        case THREAD_FINISH:;
            top->thread->state = THREAD_DEAD;
            break;
        case THREAD_JOIN:;
            top->thread->state = THREAD_SLEEPING;
            break;
        default:;
            break;
    }
}

static void
csystem_virtually_apply_transition(concurrent_system_ref ref, transition_ref transition, bool execute_resource_init_ops)
{
    // TODO: Report undefined behavior here

    switch (transition->operation.type) {
        case MUTEX:;
            csystem_virtually_apply_mutex_operation(ref, &transition->operation.mutex_operation, execute_resource_init_ops);
            break;
        case THREAD_LIFECYCLE:;
            csystem_virtually_apply_thread_operation(ref, &transition->operation.thread_operation, execute_resource_init_ops);
            break;
        default:
            mc_unimplemented();
    }
}

static void
csystem_virtually_revert_mutex_operation(concurrent_system_ref ref, mutex_operation_ref mutop)
{
    pthread_mutex_t *mutex = mutop->mutex.mutex;

    switch (mutop->type) {
        case MUTEX_INIT:;

            // Remove the mutex ONLY if we know it didn't already
            // exist. The `MUTEX_UNKNOWN` state defines any mutex whose
            // state is undefined. This differs from `MUTEX_DESTROYED`,
            // which is assigned after being destroyed. The contents
            // are still undefined, but we know why they are in the latter
            // case whereas in the former we don't. This distinction allows
            // use to tell when we have undefined behavior with an existing
            // mutex vs a new mutex entirely
            if (mutop->mutex.state == MUTEX_UNKNOWN) {

            }

            // Implicit in this assumption is this undoes the top-most
            // mutex init from the transition stack. Otherwise, we'd be
            // unregistering a mutex with a valid `mutid`. This is true
            // of all visible objects, however
            hash_table_remove(ref->mutex_map, mutex);
            ref->mut_next--;
            break;

//            fallthrough; // Intentional fallthrough
        default:;
            mutex_ref shadow_mutex = csystem_get_mutex_with_pthread(ref, mutex);
            *shadow_mutex = mutop->mutex;
            break;
    }
}

static void
csystem_virtually_revert_thread_operation(concurrent_system_ref ref, thread_operation_ref top)
{
    switch (top->type) {
        case THREAD_CREATE:;
            // Remove the new thread that was created
            ref->tid_next--;
            break;
        case THREAD_FINISH:
        case THREAD_JOIN:
            // TODO: Ensure that the only way to reach
            // THREAD_FINISH AND THREAD_JOIN is indeed THREAD_ALIVE
            top->thread->state = THREAD_ALIVE;
            break;
        case THREAD_TERMINATE_PROCESS:;
            mc_unimplemented();
        default:;
            break;
    }
}

static void
csystem_virtually_revert_transition(concurrent_system_ref ref, transition_ref transition)
{
    dynamic_transition_ref was_next = csystem_get_transition_slot_for_thread(ref, transition->thread);
    *was_next = transition_convert_to_dynamic_transition_in_system(ref, transition); /* t_top represents what executed BEFORE to reach the current next transition */

    switch (transition->operation.type) {
        case MUTEX:;
            csystem_virtually_revert_mutex_operation(ref, &transition->operation.mutex_operation);
            break;
        case THREAD_LIFECYCLE:;
            csystem_virtually_revert_thread_operation(ref, &transition->operation.thread_operation);
            break;
        default:
            mc_unimplemented();
    }
}

static state_stack_item_ref
csystem_grow_state_stack(concurrent_system_ref ref)
{
    state_stack_item_ref s_top = &ref->s_stack[++ref->s_stack_top];
    s_top->backtrack_set = hash_set_create((hash_function) thread_hash, (hash_equality_function) threads_equal);
    s_top->done_set = hash_set_create((hash_function) thread_hash, (hash_equality_function) threads_equal);
    mc_assert(ref->s_stack_top <= MAX_VISIBLE_OPERATION_DEPTH);
    return s_top;
}

static state_stack_item_ref
csystem_grow_state_stack_by_running_thread(concurrent_system_ref ref, thread_ref advancing_thread)
{
    mc_assert(ref->s_stack_top >= 0);
    {
        state_stack_item_ref s_top_old = &ref->s_stack[ref->s_stack_top];
        bool inserted = hash_set_insert(s_top_old->done_set, advancing_thread);
//        mc_assert(inserted);
    }
    return csystem_grow_state_stack(ref);
}

inline state_stack_item_ref
csystem_shrink_state_stack(concurrent_system_ref ref)
{
    mc_assert(ref->s_stack_top >= 0);
    state_stack_item_ref s_top = &ref->s_stack[ref->s_stack_top--];
    return s_top;
}

inline transition_ref
csystem_grow_transition_stack_by_running_thread(concurrent_system_ref ref, thread_ref thread)
{
    transition_ref t_top = &ref->t_stack[++ref->t_stack_top];
    mc_assert(ref->t_stack_top <= MAX_VISIBLE_OPERATION_DEPTH);

    dynamic_transition_ref thread_runs = csystem_get_transition_slot_for_thread(ref, thread);
    transition snapshot = dynamic_transition_get_snapshot(thread_runs);
    csystem_virtually_apply_transition(ref, &snapshot, false);

    // Copy the contents of the transition into the top of the transition stack
    *t_top = snapshot;

    return t_top;
}

static inline transition_ref
csystem_replay_transition_to_restore_state_after_backtracking(concurrent_system_ref ref, transition_ref transition)
{
    transition_ref t_top = &ref->t_stack[++ref->t_stack_top];
    mc_assert(ref->t_stack_top <= MAX_VISIBLE_OPERATION_DEPTH);

    csystem_virtually_apply_transition(ref, transition, true);

    // Copy the contents of the transition into the the top of the transition stack
    *t_top = *transition;
    return t_top;
}

inline transition_ref
csystem_shrink_transition_stack(concurrent_system_ref ref)
{
    mc_assert(ref->t_stack_top >= 0);
    transition_ref t_top = &ref->t_stack[ref->t_stack_top--];
    csystem_virtually_revert_transition(ref, t_top);
    return t_top;
}

void
csystem_undo_most_recent_transition(concurrent_system_ref ref)
{
    if (csystem_transition_stack_is_empty(ref)) return;

    // This must have been what the thread
    // WAS going to execute next before it was scheduled
    // to run; otherwise, the operation that actually would
    // have run next would have been executed
    transition_ref old_t_top = csystem_shrink_transition_stack(ref);
    thread_ref thread_executed = old_t_top->thread;

    dynamic_transition_ref t_next_dest = csystem_get_transition_slot_for_tid(ref, thread_executed->tid);
    *t_next_dest = transition_convert_to_dynamic_transition_in_system(ref, old_t_top);

//    // Exit early for a thread_start operation (nothing can execute before it)
//    if (old_t_top->operation.type == THREAD_LIFECYCLE) {
//        thread_operation_ref top = &old_t_top->operation.thread_operation;
//        if (top->type == THREAD_START) return;
//    }
//
//    // Find the first transition in the transition stack that
//    // this thread executed (aside from the `old_next_for_tid`)
//    thread_ref thread_executed = old_t_top->thread;
//    const int t_stack_top = ref->t_stack_top;
//    for (int i = t_stack_top; i >= 0; i--) {
//        transition_ref t_i = &ref->t_stack[i];
//        if (threads_equal(t_i->thread, thread_executed)) {
//            dynamic_transition_ref t_next_dest = csystem_get_transition_slot_for_tid(ref, thread_executed->tid);
//            *t_next_dest = transition_convert_to_dynamic_transition_in_system(ref, t_i);
//            return;
//        }
//    }
}

state_stack_item_ref
csystem_pop_program_stacks_for_backtracking(concurrent_system_ref ref)
{
    mc_assert(!csystem_state_stack_is_empty(ref));
    state_stack_item_ref s_old_top = csystem_shrink_state_stack(ref);
    csystem_undo_most_recent_transition(ref);
    return s_old_top;
}

inline dynamic_transition_ref
csystem_get_transition_slot_for_thread(concurrent_system_ref ref, csystem_local thread_ref thread)
{
    return csystem_get_transition_slot_for_tid(ref, thread->tid);
}

inline dynamic_transition_ref
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
        dynamic_transition_ref tid_transition = csystem_get_transition_slot_for_tid(ref, tid);
        transition tid_snapshot = dynamic_transition_get_snapshot(tid_transition);
        if (transition_enabled(&tid_snapshot)) {
            tref_array[enabled_thread_count++] = tid_snapshot;
        }
    }
    return enabled_thread_count;
}

inline transition_ref
csystem_transition_stack_get_element(concurrent_system_ref ref, int i)
{
    mc_assert(i >= 0 && i < MAX_VISIBLE_OPERATION_DEPTH);
    return &ref->t_stack[i];
}

inline transition_ref
csystem_transition_stack_top(concurrent_system_ref ref)
{
    if (csystem_transition_stack_is_empty(ref)) return NULL;
    return csystem_transition_stack_get_element(ref, ref->t_stack_top);
}

inline state_stack_item_ref
csystem_state_stack_top(concurrent_system_ref ref)
{
    mc_assert(ref->s_stack_top >= 0 && ref->s_stack_top < MAX_VISIBLE_OPERATION_DEPTH);
    return &ref->s_stack[ref->s_stack_top];
}

inline state_stack_item_ref
csystem_state_stack_get_element(concurrent_system_ref ref, int i)
{
    mc_assert(i >= 0 && i < MAX_VISIBLE_OPERATION_DEPTH);
    return &ref->s_stack[i];
}

void
csystem_copy_per_thread_transitions(concurrent_system_ref ref, transition_ref tref_array)
{
    int thread_count = csystem_get_thread_count(ref);
    for (tid_t tid = 0; tid < thread_count; tid++) {
        dynamic_transition_ref tid_transition = csystem_get_transition_slot_for_tid(ref, tid);
        tref_array[tid] = dynamic_transition_get_snapshot(tid_transition);
    }
}

void
csystem_replace_per_thread_transitions_after_backtracking(concurrent_system_ref ref, transition_ref tref_array)
{
    // TODO: See if we can make this a bit more efficient (perhaps with a memcpy)
    int thread_count = csystem_get_thread_count(ref);
    for (tid_t tid = 0; tid < thread_count; tid++) {
        dynamic_transition_ref tid_dynamic_transition = csystem_get_transition_slot_for_tid(ref, tid);
        transition_ref transition_by_tid = &tref_array[tid];
        *tid_dynamic_transition = transition_convert_to_dynamic_transition_in_system(ref, transition_by_tid);
    }
}

void
csystem_copy_per_thread_dynamic_transitions(concurrent_system_ref ref, dynamic_transition_ref tref_array)
{
    int thread_count = csystem_get_thread_count(ref);
    size_t cpy_size = thread_count * sizeof(*tref_array);

    // NOTE: Assumes tref_array maps tid_t to the
    // transition corresponding to that thread
    memcpy(tref_array, ref->t_next, cpy_size);
}

static void
csystem_replace_per_thread_transitions_for_backtracking(concurrent_system_ref ref, dynamic_transition_ref tref_array)
{
    int thread_count = csystem_get_thread_count(ref);
    size_t cpy_size = thread_count * sizeof(*tref_array);

    // NOTE: Assumes tref_array maps tid_t to the
    // transition corresponding to that thread
    memcpy(ref->t_next, tref_array, cpy_size);
}

dynamic_transition_ref
csystem_get_first_enabled_transition(concurrent_system_ref ref)
{
    for (int i = 0; i < ref->tid_next; i++) {
        dynamic_transition_ref t_next_i = &ref->t_next[i];
        if (dynamic_transition_enabled(t_next_i)) return t_next_i;
    }
    return NULL;
}

static dynamic_transition_ref
csystem_get_first_enabled_transition_in_backtrack_set(concurrent_system_ref ref, state_stack_item_ref ss_item)
{
    hash_set_iter_ref iter = hash_set_iter_create(ss_item->backtrack_set);
    while (hash_set_iter_has_next(iter)) {
        hash_set_entry ent = hash_set_iter_get_next(iter);
        thread_ref backtrack_thread = ent.value;
        dynamic_transition_ref dynamic_transition_for_thread = csystem_get_transition_slot_for_thread(ref, backtrack_thread);
        if (dynamic_transition_enabled(dynamic_transition_for_thread)) {
            hash_set_iter_destroy(iter);
            return dynamic_transition_for_thread;
        }
    }
    hash_set_iter_destroy(iter);
    return NULL;
}

dynamic_transition_ref
csystem_pop_first_enabled_transition_in_backtrack_set(concurrent_system_ref ref, state_stack_item_ref ss_item)
{
    dynamic_transition_ref next_transition = csystem_get_first_enabled_transition_in_backtrack_set(ref, ss_item);
    if (next_transition == NULL) return NULL;
    hash_set_remove(ss_item->backtrack_set, next_transition->thread);
    hash_set_insert(ss_item->done_set, next_transition->thread);
    return next_transition;
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
    ref->detached_t_top = ref->t_stack_top;
    ref->detached_s_top = ref->s_stack_top;
}

void
csystem_end_backtrack(concurrent_system_ref ref)
{
    mc_assert(ref->is_backtracking);
    ref->is_backtracking = false;

    // We only expect to look backwards. Looking forward
    // makes no sense when backtracking. "Back" is in the name
    // after all...
    mc_assert(ref->t_stack_top < ref->detached_t_top);
    mc_assert(ref->s_stack_top < ref->detached_s_top);

    const bool no_transitions_left = csystem_transition_stack_is_empty(ref);
    const int cur_top = no_transitions_left ? 0 : ref->t_stack_top;
    const int dest_top = ref->detached_t_top;

    // NOTE: The assumption is that while
    // is_backtracking = true, we don't clear out the
    // contents of the array t_stack. If, in the future,
    // this *is* done (it shouldn't be done), this code will break. The solution
    // is to simply check if we're backtracking in the spot
    // where clearing the stack was added and don't clear
    // it out in the case we are backtracking
    for (int i = cur_top; i <= dest_top; i++) {
        transition_ref transition_i = &ref->t_stack[i];
        csystem_replay_transition_to_restore_state_after_backtracking(ref, transition_i);
    }

    ref->s_stack_top = ref->detached_s_top;
    ref->detached_t_top = -1;
    ref->detached_s_top = -1;
}

bool
happens_before(concurrent_system_ref ref, int i, int j)
{
    mc_assert(i > 0 && j > 0);
    mc_assert(!ref->is_backtracking || ref->detached_t_top >= 0);

    const int top = csystem_get_top_of_transition_stack_based_on_context(ref);

    if (i > j) return false;
    {
        transition_ref t_i = &ref->t_stack[i];
        transition_ref t_j = &ref->t_stack[j];
        if (transitions_dependent(t_i, t_j))
            return true;
    }

    // TODO: Make dfs stack a `static` variable and alloc
    // when necessary. Significantly reduces free and malloc calls

    // DFS is used instead of BFS since the
    // performance of removing items from
    // the array is O(1) for items at the end of the array
    array_ref dfs_stack = array_create();

    // Reduces calls to realloc()
    array_prealloc(dfs_stack, i); // TODO: Pick a reasonable estimate here.

    // Note: We use `(void*) int-type` to be able to store in the
    // array (which expects dynamic types)
    array_append(dfs_stack, (void*)(uint64_t)i);

    while (!array_is_empty(dfs_stack)) {
        uint64_t t_search = (uint64_t) array_remove_last(dfs_stack);
        transition_ref t_search_ref = &ref->t_stack[t_search];

        if (t_search == j) {
            array_destroy(dfs_stack, NULL);
            return true;
        }
        else if (t_search > j) {
            // We only increase from here, but we may have pushed something onto the stack that is less than j, so we have to search it
            continue;
        }
        else {
            for (int k = t_search + 1; k < top; k++) {
                transition_ref t_k = &ref->t_stack[k];
                if (transitions_dependent(t_k, t_search_ref))
                    array_append(dfs_stack, (void*)(uint64_t)k);
            }
        }
    }
    array_destroy(dfs_stack, NULL);
    return false;
}

bool
happens_before_thread(concurrent_system_ref ref, int i, thread_ref p)
{
    thread_ref t_i = proc(&ref->t_stack[i]);
    if (threads_equal(p, t_i)) return true;

    const int t_stack_top = csystem_get_top_of_transition_stack_based_on_context(ref);
    for (int k = i; k <= t_stack_top; k++) {
        transition_ref S_k = &ref->t_stack[k];

        // Check threads_equal first (much less costly than happens before)
        if (threads_equal(p, S_k->thread) && happens_before(ref, i, k))
            return true;
    }
    return false;
}

bool
csystem_p_q_could_race(concurrent_system_ref ref, int i, thread_ref q, thread_ref p)
{
    mc_assert(ref->is_backtracking);

    const int t_stack_top = csystem_get_top_of_transition_stack_based_on_context(ref);
    for (int j = i + 1; j <= t_stack_top; j++) {
        transition_ref S_j = &ref->t_stack[j];
        if (threads_equal(q, proc(S_j)) && happens_before_thread(ref, j, p))
            return true;
    }
    return false;
}

void
csystem_dynamically_update_backtrack_sets(concurrent_system_ref ref)
{
    // TODO: This function needs to be split into smaller pieces
    csystem_start_backtrack(ref);

    // next(last(S), p) for each thread
    static dynamic_transition dynamic_transitions_at_last_s[MAX_TOTAL_THREADS_PER_SCHEDULE];
    static transition static_transitions_at_last_s[MAX_TOTAL_THREADS_PER_SCHEDULE];
    static transition scratch_enabled_at_i[MAX_TOTAL_THREADS_PER_SCHEDULE];
    csystem_copy_per_thread_transitions(ref, static_transitions_at_last_s);
    csystem_copy_per_thread_dynamic_transitions(ref, dynamic_transitions_at_last_s);

    int thread_count = csystem_get_thread_count(ref);
    int t_stack_height = csystem_transition_stack_count(ref);
    mc_assert(thread_count <= MAX_TOTAL_THREADS_PER_SCHEDULE);
    mc_assert(t_stack_height <= MAX_VISIBLE_OPERATION_DEPTH);

    // TODO: Make the transition map static
    hash_table_ref thread_to_transition_map_at_last_s = hash_table_create((hash_function) thread_hash, (hash_equality_function) threads_equal);
    for (tid_t tid = 0; tid < thread_count; tid++) {
        thread_ref tid_thread = csystem_get_thread_with_tid(ref, tid);
        transition_ref transition_for_tid = &static_transitions_at_last_s[tid];
        hash_table_set(thread_to_transition_map_at_last_s, tid_thread, transition_for_tid);
    }

    /* Don't need to keep going further into the transition stack if everything is processed */
    for (int i = t_stack_height - 1; i >= 0 && !hash_table_is_empty(thread_to_transition_map_at_last_s); i--) {
        transition_ref S_i = csystem_transition_stack_top(ref);
        csystem_pop_program_stacks_for_backtracking(ref); // Ensures that we are looking at the enabled threads at pre(S, i)
        state_stack_item_ref state_stack_item_i = csystem_state_stack_get_element(ref, i); // pre(S, i), now the top of the state stack

        next_thread: for (int tid = 0; tid < thread_count; tid++) {
            thread_ref p = csystem_get_thread_with_tid(ref, tid);

            // Transition in the latest state stack (at this point lost but whose enabled transitions were copied into the
            // array `enabled_transitions_s_top`)
            transition_ref p_transition = NULL;
            if ((p_transition = hash_table_get(thread_to_transition_map_at_last_s, p)) != NULL) {
                mc_assert(p_transition->thread == p);

                bool found_max_i_condition = transitions_coenabled(S_i, p_transition) &&
                                             transitions_dependent(S_i, p_transition) &&
                                             !happens_before_thread(ref, i, p);
                if (found_max_i_condition) {
                    int num_enabled_at_s_i = csystem_copy_enabled_transitions(ref, scratch_enabled_at_i);
                    for (int s_i_index = 0; s_i_index < num_enabled_at_s_i; s_i_index++) {
                        transition_ref q_transition = &scratch_enabled_at_i[s_i_index];
                        thread_ref q = q_transition->thread;

                        bool q_is_in_E = threads_equal(q, p) || csystem_p_q_could_race(ref, i, q, p);
                        if (q_is_in_E) {
                            hash_set_insert(state_stack_item_i->backtrack_set, q);

                            // TODO: Only one q is needed. An optimization
                            // where we check the done set for q first would
                            // reduce backtracking

                            // The thread doesn't need to be checked again: any other k
                            // for which the condition is satisfied will be less (or
                            hash_table_remove(thread_to_transition_map_at_last_s, p);
                            goto next_thread;
                        }
                    }

                    // If we exit the loop, we know that `E` is the empty set. In this case, we must
                    // add every enabled thread to the backtrack set
                    for (int s_i_index = 0; s_i_index < num_enabled_at_s_i; s_i_index++) {
                        transition_ref q_transition = &scratch_enabled_at_i[s_i_index];
                        thread_ref q = q_transition->thread;
                        hash_set_insert(state_stack_item_i->backtrack_set, q);
                    }
                }
            }
        }
    }
    hash_table_destroy(thread_to_transition_map_at_last_s);
    csystem_end_backtrack(ref);
    csystem_replace_per_thread_transitions_after_backtracking(ref, static_transitions_at_last_s);
}

static void
dpor_init_thread_transition(dynamic_transition_ref tref, thread_ref thread, thread_operation_type type)
{
    thread_operation top;
    top.type = type;
    top.thread = thread;
    tref->operation.type = THREAD_LIFECYCLE;
    tref->operation.thread_operation = top;
    tref->thread = thread;
}

inline void
dpor_init_thread_start_transition(dynamic_transition_ref transition, thread_ref thread)
{
    dpor_init_thread_transition(transition, thread, THREAD_START);
}

inline void
dpor_init_thread_finish_transition(dynamic_transition_ref transition, thread_ref thread)
{
    dpor_init_thread_transition(transition, thread, THREAD_FINISH);
}

void
csystem_print_transition_stack(concurrent_system_ref ref)
{
    puts("*** TRANSITION STACK CONTENT DUMP ****");
    for (int i = 0; i < ref->t_stack_top; i++) {
        transition_ref t_i = &ref->t_stack[i];
        transition_pretty(t_i);
    }
    puts("***************");
}

void
mutex_transition_convert_to_dynamic_transition(concurrent_system_ref ref, mutex_operation_ref tref, dynamic_mutex_operation_ref dmref)
{
    dmref->type = tref->type;
    dmref->mutex = csystem_get_mutex_with_mutid(ref, tref->mutex.mutid);
}

void
thread_transition_convert_to_dynamic_transition(concurrent_system_ref ref, thread_operation_ref tref, dynamic_thread_operation_ref dmref)
{
    dmref->type = tref->type;
    dmref->thread = tref->thread;
}


dynamic_transition
transition_convert_to_dynamic_transition_in_system(concurrent_system_ref ref, transition_ref tref)
{
    dynamic_transition d;
    d.thread = tref->thread;
    d.operation.type = tref->operation.type;
    switch (tref->operation.type) {
        case MUTEX:
            mutex_transition_convert_to_dynamic_transition(ref, &tref->operation.mutex_operation, &d.operation.mutex_operation);
            break;
        case THREAD_LIFECYCLE:
            thread_transition_convert_to_dynamic_transition(ref, &tref->operation.thread_operation, &d.operation.thread_operation);
            break;
        default:
            mc_unimplemented();
            break;
    }
    return d;
}