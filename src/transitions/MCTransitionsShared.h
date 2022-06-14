#ifndef MC_MCTRANSITIONSSHARED_H
#define MC_MCTRANSITIONSSHARED_H

#include <pthread.h>
#include <typeinfo>
#include "MCShared.h"
#include "MCMINI_Private.h"
#include "transitions/threads/MCThreadDefs.h"
#include "transitions/mutex/MCMutexDefs.h"
#include "transitions/semaphore/MCSemaphoreDefs.h"
#include "transitions/barrier/MCBarrierDefs.h"
#include "transitions/cond/MCCondDefs.h"
#include "transitions/misc/MCMiscDefs.h"

extern "C" {
#include "transitions/wrappers/MCSharedLibraryWrappers.h"
}

/* Source program thread control */
template<typename SharedMemoryData> void
thread_post_visible_operation_hit(const std::type_info &type, SharedMemoryData * shmData)
{
    auto newTypeInfo = MCSharedTransition(tid_self, type);
    auto newShmData = shmData;
    memcpy(shmTransitionTypeInfo, &newTypeInfo, sizeof(MCSharedTransition));
    memcpy(shmTransitionData, newShmData, sizeof(SharedMemoryData));
}

void thread_post_visible_operation_hit(const std::type_info &type);

void thread_await_mc_scheduler();
void thread_await_mc_scheduler_for_thread_start_transition();
void thread_awake_mc_scheduler_for_thread_finish_transition();
void mc_exit_main_thread();


#endif //MC_MCTRANSITIONSSHARED_H
