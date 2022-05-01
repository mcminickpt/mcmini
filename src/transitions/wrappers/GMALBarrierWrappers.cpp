#include "GMALBarrierWrappers.h"
#include "transitions/GMALTransitionsShared.h"
#include "transitions/GMALBarrierInit.h"
#include "transitions/GMALBarrierWait.h"

extern "C" {
    #include "transitions/wrappers/GMALSharedLibraryWrappers.h"
}

int
gmal_pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count)
{
    auto newlyCreatedShadow = GMALBarrierShadow(barrier, count);
    thread_post_visible_operation_hit<GMALBarrierShadow>(typeid(GMALBarrierInit), &newlyCreatedShadow);
    thread_await_gmal_scheduler();
    return __real_pthread_barrier_init(barrier, attr, count);
}

int
gmal_pthread_barrier_wait(pthread_barrier_t *barrier)
{
    // It doesn't matter what count we pass here it is discarded
    // IMPROVEMENT: Simply pass the barrier in shared memory and do
    // a corresponding read on the other side. This can be done in
    // other wrapper functions as well
    auto newlyCreatedShadow = GMALBarrierShadow(barrier, 0);
    thread_post_visible_operation_hit<GMALBarrierShadow>(typeid(GMALBarrierWait), &newlyCreatedShadow);
    thread_await_gmal_scheduler();
    return __real_pthread_barrier_wait(barrier);
}