#include "GMALSemaphoreTransitionWrappers.h"
#include "transitions/GMALTransitionsShared.h"
#include "transitions/GMALSemWait.h"
#include "transitions/GMALSemPost.h"
#include "transitions/GMALSemInit.h"

extern "C" {
    #include "GMALSharedLibraryWrappers.h"
}

int
gmal_sem_init(sem_t *sem, int pshared, unsigned int count)
{
    // TODO: We don't support shared semaphores
    GMAL_ASSERT(pshared == 0);
    auto newlyCreatedSemaphore = GMALSemaphoreShadow(sem, count);
    thread_post_visible_operation_hit<GMALSemaphoreShadow>(typeid(GMALSemInit), &newlyCreatedSemaphore);
    thread_await_gmal_scheduler();
    return __real_sem_init(sem, pshared, count);
}

int
gmal_sem_post(sem_t *sem)
{
    thread_post_visible_operation_hit<sem_t*>(typeid(GMALSemPost), &sem);
    thread_await_gmal_scheduler();
    return __real_sem_post(sem);
}

int
gmal_sem_wait(sem_t *sem)
{
    thread_post_visible_operation_hit<sem_t*>(typeid(GMALSemEnqueue), &sem);
    thread_await_gmal_scheduler();

    thread_post_visible_operation_hit<sem_t*>(typeid(GMALSemWait), &sem);
    thread_await_gmal_scheduler();
    return __real_sem_wait(sem);
}
