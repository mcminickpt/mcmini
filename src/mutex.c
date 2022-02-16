#include "mutex.h"


mutex_ref
mutex_create(pthread_mutex_t *mutex)
{
    if (!mutex) return NULL;
    mutex_ref ref = malloc(sizeof(*ref));
    if (ref) {
        ref->mutex = mutex;
        ref->owner = NULL;
        ref->state = MUTEX_UNKNOWN;
    }
    return ref;
}

mutex_ref
mutex_copy(mutex_refc other)
{
    mutex_ref cpy = mutex_create(other->mutex);
    if (cpy) {
        cpy->state = other->state;
        cpy->owner = thread_copy(other->owner);
    }
    return cpy;
}

void
mutex_destroy(mutex_ref ref)
{
    if (!ref) return;
    free(ref);
}

bool
mutexes_equal(mutex_ref m1, mutex_ref m2)
{
    if (!m1) return m2 == NULL;
    if (!m2) return m1 == NULL;
    return m1->mutex == m2->mutex;
}

bool
mutex_operations_race(mutex_operation_refc mutop1, mutex_operation_refc mutop2)
{
    if (!mutop1 || !mutop2) return NULL;
    return mutexes_equal(mutop1->mutex, mutop2->mutex);
}

bool
mutex_operation_enabled(mutex_operation_refc mop, thread_ref thread)
{
    if (!mop || !thread) return false;

    switch (mop->type) {
        case MUTEX_LOCK:
            return mop->mutex->state != MUTEX_LOCKED;
        default:
            return true;
    }
}