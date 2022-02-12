#include "dpor_methods.h"

int
transitions_dependent(struct transition t1, struct transition t2)
{
    // Execution by the same thread
    if (threads_equal(t1.thread, t2.thread))
        return true;

    // t1 creates the thread executing t2 or vice-versa
    if (transition_creates(t1, t2.thread) || transition_creates(t2, t1.thread))
        return true;

    // t1 or t2 is a join operation
    // It is not true that for all states both the thread join
    // and the other operation, whatever it is, are both enabled
    if (t1.operation.type == THREAD_LIFECYCLE
        && t1.operation.thread_operation.type == THREAD_JOIN)
        return true;
    if (t2.operation.type == THREAD_LIFECYCLE
        && t2.operation.thread_operation.type == THREAD_JOIN)
        return true;

    // Mutex operations on the same mutexes
    if (t1.operation.type == MUTEX && t2.operation.type == MUTEX)
        return mutex_operations_race(t1.operation.mutex_operation, t2.operation.mutex_operation);

    return false;
}

void
dynamically_update_backtrack_sets(struct state_stack_item ss_item)
{
    struct shared_state wrapped = ss_item.state;

    // 1.

    // Iterate through the threads to get
    // the set of transitions and threads
    // that are enabled
    //
    // wrapped.threads;
    // wrapped.transition_map -> to be able to get the next transition
    // for each thread

    // 2.

}