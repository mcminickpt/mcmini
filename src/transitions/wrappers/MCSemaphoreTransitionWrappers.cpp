#include "MCSemaphoreTransitionWrappers.h"
#include "transitions/MCTransitionsShared.h"
#include "transitions/semaphore/MCSemWait.h"
#include "transitions/semaphore/MCSemPost.h"
#include "transitions/semaphore/MCSemInit.h"
#include "transitions/semaphore/MCSemEnqueue.h"

extern "C" {
#include "MCSharedLibraryWrappers.h"
}

int
mc_sem_init(sem_t *sem, int pshared, unsigned int count)
{
    // TODO: We don't support shared semaphores
    MC_ASSERT(pshared == 0);
    auto newlyCreatedSemaphore = MCSemaphoreShadow(sem, count);
    thread_post_visible_operation_hit<MCSemaphoreShadow>(typeid(MCSemInit), &newlyCreatedSemaphore);
    thread_await_mc_scheduler();
    return __real_sem_init(sem, pshared, count);
}

int
mc_sem_post(sem_t *sem)
{
    thread_post_visible_operation_hit<sem_t*>(typeid(MCSemPost), &sem);
    thread_await_mc_scheduler();
    return __real_sem_post(sem);
}

int
mc_sem_wait(sem_t *sem)
{
    thread_post_visible_operation_hit<sem_t*>(typeid(MCSemEnqueue), &sem);
    thread_await_mc_scheduler();

    thread_post_visible_operation_hit<sem_t*>(typeid(MCSemWait), &sem);
    thread_await_mc_scheduler();
    return __real_sem_wait(sem);
}