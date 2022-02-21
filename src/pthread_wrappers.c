#include "pthread_wrappers.h"

int
dpor_pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
    thread tself = thread_get_self();
    mutex_operation init_mutex;
    shm_visible_operation visible;

    // TODO: Complete initialization here
    init_mutex.type = MUTEX_INIT;
    visible.type = MUTEX;
    visible.mutex_operation = init_mutex;

    shm_child_result.shm_transition->thread = tself;
    shm_child_result.shm_transition->operation = visible;

    thread_await_dpor_scheduler();
    return pthread_mutex_init(mutex, attr);
}