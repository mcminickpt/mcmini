#include "pthread_wrappers.h"
#include "mutex.h"
#include "fail.h"
#include <stdio.h>
#include "thread.h"

struct dpor_thread_routine_arg {
    void *arg;
    thread_routine routine;
};

static void *
dpor_thread_routine_wrapper(void * arg)
{

}

static void
dpor_post_mutex_operation_to_parent(pthread_mutex_t *m, mutex_operation_type type, mutex_state state)
{
    thread_ref tself = thread_get_self();
    shm_mutex_operation init_mutex;
    mutex new_mutex = {
            .mutex = m,
            .owner = NULL,
            .state = state
    };
    shm_visible_operation to_parent;
    init_mutex.type = type;
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

int
dpor_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*routine) (void *), void *arg)
{
    mc_assert(attr == NULL);

    struct dpor_thread_routine_arg *dpor_thread_arg = malloc(sizeof(struct dpor_thread_routine_arg));

    int pthread_create_return = pthread_create(thread, attr, routine, arg);

    return pthread_create_return;
}
