#include "pthread_wrappers.h"
#include "mutex.h"
#include <stdio.h>

int
dpor_pthread_mutex_init(pthread_mutex_t *m, pthread_mutexattr_t *attr)
{
    thread_ref tself = thread_get_self();
    shm_mutex_operation init_mutex;
    mutex new_mutex = {
            .mutex = m,
            .owner = tself,
            .state = MUTEX_UNLOCKED
    };
    shm_visible_operation to_parent;
    init_mutex.type = MUTEX_INIT;
    init_mutex.mutex = new_mutex;
    to_parent.type = MUTEX;
    to_parent.mutex_operation = init_mutex;
    shm_child_result->thread = *tself;
    shm_child_result->operation = to_parent;
    printf("CHILD HIT THIS\n");
    thread_await_dpor_scheduler();
    return pthread_mutex_init(m, attr);
}

//int
//dpor_pthread_mutex_lock(pthread_mutex_t *mutex)
//{
//    thread_ref tself = thread_get_self();
//
//
//    mutex_operation init_mutex;
//    shm_visible_operation visible;
//
//    // TODO: Complete initialization here
//    init_mutex.type = MUTEX_INIT;
//    visible.type = MUTEX;
//    visible.mutex_operation = init_mutex;
//
//    shm_child_result->thread = *tself;
//    shm_child_result->operation = visible;
//
//    thread_await_dpor_scheduler();
//}
//
//int
//dpor_pthread_mutex_unlock(pthread_mutex_t *mutex)
//{
//
//}
//
//int
//dpor_pthread_mutex_destroy(pthread_mutex_t *mutex)
//{
//
//}
//
//int
//dpor_pthread_create(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *)
//{
//
//}
//
//int
//dpor_pthread_join(pthread_t *, void**)
//{
//
//}