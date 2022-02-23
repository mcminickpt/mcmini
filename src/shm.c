#include "shm.h"
#include "mutex.h"
#include "dpor.h"

transition_ref
create_transition_from_shm(shm_transition_ref shm_ref)
{
    if (!shm_ref) return NULL;

    transition_ref new = transition_alloc();
    tid_t shmtid = shm_ref->thread.tid;
    new->thread = &threads[shmtid];

    visible_operation_type type = shm_ref->operation.type;
    switch (type) {
        case MUTEX:;
            mutex_operation_ref mutop = mutex_operation_alloc();
            shm_mutex_operation_ref shmmop = &shm_ref->operation.mutex_operation;
            mutop->mutex = mutex_create(shmmop->mutex.mutex);
            mutop->type = shmmop->type;
            break;
        case THREAD_LIFECYCLE:;
            thread_operation_ref top = thread_operation_alloc();
            shm_thread_operation_ref shmtop = &shm_ref->operation.thread_operation;
            top->thread = &threads[shmtop->thread.tid];
            top->type = shmtop->type;
            break;
        default:
            return NULL;
    }
    return new;
}