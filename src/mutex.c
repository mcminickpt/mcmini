#include "mutex.h"

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