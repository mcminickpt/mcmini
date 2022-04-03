#include "pthread_wrappers.h"
#include "mutex.h"
#include "fail.h"
#include <stdio.h>
#include "thread.h"

STRUCT_DECL(dpor_thread_routine_arg)
struct dpor_thread_routine_arg {
    void *arg;
    thread_routine routine;
};

static void
dpor_post_mutex_operation_to_parent(pthread_mutex_t *m, mutex_operation_type type)
{
    thread_ref tself = thread_get_self();
    shm_mutex_operation init_mutex;
    shm_visible_operation to_parent;
    init_mutex.type = type;
    init_mutex.mutex = m;
    to_parent.type = MUTEX;
    to_parent.mutex_operation = init_mutex;
    shm_child_result->thread = *tself;
    shm_child_result->operation = to_parent;
}

static void
dpor_post_thread_operation_to_parent_with_target(tid_t tid, thread_operation_type type, pthread_t target)
{
    thread_ref tself = thread_get_self();
    shm_thread_operation init_thread;
    shm_visible_operation to_parent;
    init_thread.type = type;
    init_thread.tid = tid;
    init_thread.target = target;
    to_parent.type = THREAD_LIFECYCLE;
    to_parent.thread_operation = init_thread;
    shm_child_result->thread = *tself;
    shm_child_result->operation = to_parent;
}

static void
dpor_post_thread_operation_to_parent(tid_t tid, thread_operation_type type)
{
    dpor_post_thread_operation_to_parent_with_target(tid, type, pthread_self());
}

static void *
dpor_thread_routine_wrapper(void * arg)
{
    csystem_register_thread(&csystem);
    sem_post(&dpor_pthread_create_binary_sem);

    dpor_thread_routine_arg_ref unwrapped_arg = (dpor_thread_routine_arg_ref)arg;

    // Simulates being blocked at thread creation -> THREAD_START for this thread
    thread_await_dpor_scheduler_for_thread_start_transition();
    void * return_value = unwrapped_arg->routine(unwrapped_arg->arg);

    // Simulates being blocked after the thread exits
    dpor_post_thread_operation_to_parent(tid_self, THREAD_FINISH);
    thread_await_dpor_scheduler();

    free(arg); // See where the thread_wrapper is created. The memory is malloc'ed and should be freed

    dpor_post_thread_operation_to_parent(tid_self, THREAD_FINISH);
    thread_awake_dpor_scheduler_for_thread_finish_transition();
    return return_value;
}

int
dpor_pthread_mutex_init(pthread_mutex_t *m, pthread_mutexattr_t *attr)
{
    dpor_post_mutex_operation_to_parent(m, MUTEX_INIT);
    thread_await_dpor_scheduler();
    return pthread_mutex_init(m, attr);
}

int
dpor_pthread_mutex_lock(pthread_mutex_t *m)
{
    dpor_post_mutex_operation_to_parent(m, MUTEX_LOCK);
    thread_await_dpor_scheduler();
    return pthread_mutex_lock(m);
}

int
dpor_pthread_mutex_unlock(pthread_mutex_t *m)
{
    dpor_post_mutex_operation_to_parent(m, MUTEX_UNLOCK);
    thread_await_dpor_scheduler();
    return pthread_mutex_unlock(m);
}

int
dpor_pthread_mutex_destroy(pthread_mutex_t *m)
{
    dpor_post_mutex_operation_to_parent(m, MUTEX_DESTROY);
    thread_await_dpor_scheduler();
    return pthread_mutex_destroy(m);
}

int
dpor_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*routine) (void *), void *arg)
{
    mc_assert(attr == NULL); // TODO: For now, we don't support attributes. This should be added in the future

    dpor_thread_routine_arg_ref dpor_thread_arg = malloc(sizeof(struct dpor_thread_routine_arg));
    dpor_thread_arg->arg = arg;
    dpor_thread_arg->routine = routine;

    int return_value = pthread_create(thread, attr, dpor_thread_routine_wrapper, dpor_thread_arg);

    // We need to ensure that the thread is
    // created has fully registered itself with the
    // concurrent system; otherwise, there is a race condition
    // in which two thread creates in the child might
    // not be scheduled to run until *two* steps of the scheduler
    sem_wait(&dpor_pthread_create_binary_sem);

    // pthread_create may fail to actually spawn a thread
    tid_t tid_return = return_value == 0 ? TID_INVALID : TID_PTHREAD_CREATE_FAILED;
    dpor_post_thread_operation_to_parent_with_target(tid_return, THREAD_CREATE, *thread);
    thread_await_dpor_scheduler();

    return return_value;
}

int
dpor_pthread_join(pthread_t pthread, void **result)
{
    dpor_post_thread_operation_to_parent_with_target(TID_INVALID, THREAD_JOIN, pthread);
    thread_await_dpor_scheduler();
    return pthread_join(pthread, result);
}


void
dpor_main_thread_enter_process_exit_loop(void)
{
    dpor_post_thread_operation_to_parent(tid_self, THREAD_TERMINATE_PROCESS);
    thread_await_dpor_scheduler();
}