#include "GMALBarrierWrappers.h"
#include "transitions/GMALTransitionsShared.h"
#include "transitions/barrier/GMALBarrierInit.h"
#include "transitions/barrier/GMALBarrierWait.h"
#include "transitions/barrier/GMALBarrierEnqueue.h"

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
    thread_post_visible_operation_hit<GMALBarrierShadow>(typeid(GMALBarrierEnqueue), &newlyCreatedShadow);
    thread_await_gmal_scheduler();

    thread_post_visible_operation_hit<GMALBarrierShadow>(typeid(GMALBarrierWait), &newlyCreatedShadow);
    thread_await_gmal_scheduler();

    // We don't directly call pthread_barrier_wait here since we'd have to do so for
    // ALL threads waiting on the barrier. Instead we can just feign a pthread_barrier_wait
    // call by simply preventing any threads waiting on the barrier from being scheduled
    return PTHREAD_BARRIER_SERIAL_THREAD;
}