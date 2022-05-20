#ifndef MC_MCTRANSITIONSSHARED_H
#define MC_MCTRANSITIONSSHARED_H

#include <pthread.h>
#include <typeinfo>
#include "../MCShared.h"

extern "C" {
    #include "transitions/wrappers/MCSharedLibraryWrappers.h"
}

#include "transitions/MCThreadCreate.h"
#include "transitions/MCThreadStart.h"
#include "transitions/MCThreadFinish.h"
#include "transitions/MCThreadJoin.h"
#include "transitions/MCMutexInit.h"
#include "transitions/MCMutexLock.h"
#include "transitions/MCMutexUnlock.h"
#include "transitions/MCSemInit.h"
#include "transitions/MCSemPost.h"
#include "transitions/MCSemWait.h"
#include "transitions/MCExitTransition.h"
#include "transitions/MCBarrierInit.h"
#include "transitions/MCBarrierWait.h"
#include "transitions/MCCondInit.h"
#include "transitions/MCCondSignal.h"
#include "transitions/MCCondBroadcast.h"
#include "transitions/MCCondWait.h"
#include "transitions/MCCondEnqueue.h"

#include "MC.h"

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
