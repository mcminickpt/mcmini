#include "mutex.h"

int
mutexes_equal(struct mutex m1, struct mutex m2)
{
    return m1.mutex == m2.mutex;
}

int
mutex_operations_race(struct mutex_operation mutop1, struct mutex_operation mutop2)
{
    return mutexes_equal(mutop1.mutex, mutop2.mutex);
}