#include "pthread_wrappers.h"
#include "mutex.h"
#include "fail.h"
#include <stdio.h>
#include "thread.h"

static void
dpor_post_mutex_operation_to_parent(pthread_mutex_t *m, mutex_operation_type type, mutex_state state)
{
    thread_ref tself = thread_get_self();
    shm_mutex_operation init_mutex;
    mutex new_mutex = {
            .mutex = m,
            .owner = NULL,
            .state = MUTEX_UNLOCKED
    };
    shm_visible_operation to_parent;
    init_mutex.type = MUTEX_INIT;
    init_mutex.mutex = new_mutex;
    to_parent.type = MUTEX;
    to_parent.mutex_operation = init_mutex;
    shm_child_result->thread = *tself;
    shm_child_result->operation = to_parent;
}

int
dpor_pthread_mutex_init(pthread_mutex_t *m, pthread_mutexattr_t *attr)
{
    dpor_post_mutex_operation_to_parent(m, MUTEX_INIT, MUTEX_UNLOCKED);
    thread_await_dpor_scheduler();
    return pthread_mutex_init(m, attr);
}

int
dpor_pthread_mutex_lock(pthread_mutex_t *m)
{
    dpor_post_mutex_operation_to_parent(m, MUTEX_LOCK, MUTEX_LOCKED);
    thread_await_dpor_scheduler();
    return pthread_mutex_lock(m);
}

int
dpor_pthread_mutex_unlock(pthread_mutex_t *m)
{
    dpor_post_mutex_operation_to_parent(m, MUTEX_UNLOCK, MUTEX_LOCKED);
    thread_await_dpor_scheduler();
    return pthread_mutex_unlock(m);
}

int
dpor_pthread_mutex_destroy(pthread_mutex_t *m)
{
    dpor_post_mutex_operation_to_parent(m, MUTEX_DESTROY, MUTEX_UNKNOWN);
    thread_await_dpor_scheduler();
    return pthread_mutex_unlock(m);
}

int dpor_pthread_create(pthread_t *thread_id, const pthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg) {
    thread_await_dpor_scheduler();
    if (attr) {
        fprintf(stderr,
                "The 'attr' parameter must be NULL for now.\n");
        exit("...with a type of errors that we will create");
    }
    thread *thread_ptr =
            new_thread_blocked(*thread_id); //This will have garbage thread_id copied in
    thread_ptr->owner = thread_get_self();
    thread_ptr->start_routine = start_routine;
    thread_ptr->arg = arg;
    pthread_t *thread_id_ptr = &(thread_ptr->tid);
    num_threads++; // Add 1 for the new child thread.
    int rc = __real_pthread_create(thread_id_ptr, attr,
                                   &start_routine, thread_ptr);
    // Now parent waits; when child reaches mc_sched(), it posts to wake_parent";
    __real_sem_wait(&thread_ptr[thread_get_self()].sem);
    *thread_id = *thread_id_ptr; // Was set by pthread_create
    return rc;
}

int pthread_join(pthread_t thread_id, void **retval) {
#if 0
    if ( ! mc_thread[get_thread_index(pthread_self())].is_alive ) {
    // This thread is exiting and libpthread.so:thr_start called pthread_join().
    return __real_pthread_join(thread_id, retval);
  }
#endif
    // FIXME:  pthread_exit() isn't really joining.  So, just skip us in sched.
    thread_await_dpor_scheduler();
    // Because the threads are scheduled one at a time, the mc_sched() above
    // caused this thread to sem_wait().  If this thread is scheduled again,
    // we can finish executing pthread_join().
    if (dpor_thread[thread_id].is_alive) {
        // Our child thread is still alive.  But it has not yet exited to wake us.
        exit("some message"); // Instead, wait for a schedule where child is not alive.
    } else {
        __real_sem_wait(&dpor_thread[thread_id].join_sem);
        return 0;
        // FIXME:
        // This depends on __real_pthread_exit().  But that function will
        // call pthread_mutex_block, which lands in our own wrapper.
        // Can our wrapper pass through to libpthread.so when the
        // child thread is no longer alive?
        // return __real_pthread_join(thread_id, retval);
    }
}
