#ifndef GMAL_GMALTRANSITIONSSHARED_H
#define GMAL_GMALTRANSITIONSSHARED_H

#include <pthread.h>
#include <typeinfo>
#include "GMALShared.h"
#include "GMAL.h"
#include "transitions/threads/GMALThreadDefs.h"
#include "transitions/mutex/GMALMutexDefs.h"
#include "transitions/semaphore/GMALSemaphoreDefs.h"
#include "transitions/barrier/GMALBarrierDefs.h"
#include "transitions/cond/GMALCondDefs.h"
#include "transitions/misc/GMALMiscDefs.h"

extern "C" {
#include "transitions/wrappers/GMALSharedLibraryWrappers.h"
}

/* Source program thread control */
template<typename SharedMemoryData> void
thread_post_visible_operation_hit(const std::type_info &type, SharedMemoryData * shmData)
{
    auto newTypeInfo = GMALSharedTransition(tid_self, type);
    auto newShmData = shmData;
    memcpy(shmTransitionTypeInfo, &newTypeInfo, sizeof(GMALSharedTransition));
    memcpy(shmTransitionData, newShmData, sizeof(SharedMemoryData));
}

void thread_post_visible_operation_hit(const std::type_info &type);

void thread_await_gmal_scheduler();
void thread_await_gmal_scheduler_for_thread_start_transition();
void thread_awake_gmal_scheduler_for_thread_finish_transition();
void gmal_exit_main_thread();


#endif //GMAL_GMALTRANSITIONSSHARED_H
