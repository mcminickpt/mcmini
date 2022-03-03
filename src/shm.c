#include "shm.h"
#include "fail.h"
#include "mutex.h"
#include "dpor.h"

void
copy_into_transition_from_shm(shm_transition_ref shm_ref, transition_ref tref)
{
    if (!shm_ref || !tref) return;

    tid_t shmtid = shm_ref->thread.tid;
    thread_ref shmthread = csystem_get_thread_with_tid(&csystem, shmtid);
    tref->thread = shmthread;

    visible_operation_type type = shm_ref->operation.type;
    switch (type) {
        case MUTEX:;
            shm_mutex_operation_ref shmmop = &shm_ref->operation.mutex_operation;
            tref->operation.type = MUTEX;
            tref->operation.mutex_operation.type = shmmop->type;
            tref->operation.mutex_operation.mutex = shmmop->mutex;
            break;
        case THREAD_LIFECYCLE:;
            shm_thread_operation_ref shmtop = &shm_ref->operation.thread_operation;
            tref->operation.type = THREAD_LIFECYCLE;
            tref->operation.thread_operation.type = shmtop->type;
            tref->operation.thread_operation.thread = csystem_get_thread_with_tid(&csystem, shmtop->thread.tid);
            break;
        default:
            mc_assert(false);
            return;
    }
}