#include "MCSemaphoreTransitionWrappers.h"
#include "transitions/MCTransitionsShared.h"
#include "transitions/MCSemWait.h"
#include "transitions/MCSemPost.h"
#include "transitions/MCSemInit.h"

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
    // NOTE: MCReadSemPost doesn't use the count passed by the child; any
    // value suffices
    auto newlyCreatedSemaphore = MCSemaphoreShadow(sem, 0);
    thread_post_visible_operation_hit<MCSemaphoreShadow>(typeid(MCSemPost), &newlyCreatedSemaphore);
    thread_await_mc_scheduler();
    return __real_sem_post(sem);
}

int
mc_sem_wait(sem_t *sem)
{
    // NOTE: MCReadSemWait doesn't use the count passed by the child; any
    // value suffices
    auto newlyCreatedSemaphore = MCSemaphoreShadow(sem, 0);
    thread_post_visible_operation_hit<MCSemaphoreShadow>(typeid(MCSemWait), &newlyCreatedSemaphore);
    thread_await_mc_scheduler();
    return __real_sem_wait(sem);
}
