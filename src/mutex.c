#include "mutex.h"
#include <stdio.h>

mutex_ref mutex_alloc(void);
MEMORY_ALLOC_DEF_DECL(mutex);
MEMORY_ALLOC_DEF_DECL(mutex_operation);

hash_t
mutex_hash(pthread_mutex_t *m)
{
    return (hash_t)m;
}

bool
mutexes_equal(mutex_refc m1, mutex_refc m2)
{
    if (!m1) return m2 == NULL;
    if (!m2) return m1 == NULL;
    return m1->mutex == m2->mutex;
}

bool
mutex_operations_race(mutex_operation_refc mutop1, mutex_operation_refc mutop2)
{
    if (!mutop1 || !mutop2) return NULL;
    return mutexes_equal(&mutop1->mutex, &mutop2->mutex);
}

bool
mutex_operation_enabled(mutex_operation_refc mop, thread_ref thread)
{
    if (!mop || !thread) return false;

    switch (mop->type) {
        case MUTEX_LOCK:
            return mop->mutex.state != MUTEX_LOCKED;
        default:
            return true;
    }
}

bool
mutex_operation_creates_mutex(mutex_operation_refc op, mutex_refc mut)
{
    if (!op || !mut) return false;
    if (op->type != MUTEX_INIT) return false;
    return mutexes_equal(&op->mutex, mut);
}

bool
mutex_operations_coenabled(mutex_operation_refc op1, mutex_operation_refc op2)
{
    if (!op1 || !op2) return false;

    if (mutex_operation_creates_mutex(op1, &op2->mutex)
    || mutex_operation_creates_mutex(op2, &op1->mutex))
        return false;

    if (op1->type == MUTEX_LOCK && op2->type == MUTEX_LOCK)
        return !mutexes_equal(&op1->mutex, &op2->mutex);

    return true;
}

char*
mutex_operation_transition_description(mutex_operation_refc mop)
{
    if (!mop) return NULL;

    size_t sz;
    char *description;

    switch (mop->type) {
        case MUTEX_LOCK:;

            break;
        default:
            return NULL;
    }
    return description;
}
