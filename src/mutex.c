#include "mutex.h"



struct mutex_operation {
    mutex_ref mutex;
    enum mutex_operation_type type;
};

bool
mutexes_equal(mutex_ref m1, mutex_ref m2)
{
    if (!m1) return m2 == NULL;
    if (!m2) return m1 == NULL;
    return m1->mutex == m2->mutex;
}

int
mutex_operations_race(struct mutex_operation mutop1, struct mutex_operation mutop2)
{
    return mutexes_equal(mutop1.mutex, mutop2.mutex);
}