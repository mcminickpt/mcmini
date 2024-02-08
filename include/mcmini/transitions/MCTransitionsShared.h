#ifndef INCLUDE_MCMINI_TRANSITIONS_MCTRANSITIONSSHARED_HPP
#define INCLUDE_MCMINI_TRANSITIONS_MCTRANSITIONSSHARED_HPP

#include "mcmini/MCShared.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/barrier/MCBarrierDefs.h"
#include "mcmini/transitions/cond/MCCondDefs.h"
#include "mcmini/transitions/misc/MCMiscDefs.h"
#include "mcmini/transitions/mutex/MCMutexDefs.h"
#include "mcmini/transitions/rwlock/MCRWLockDefs.h"
#include "mcmini/transitions/rwwlock/MCRWWLockDefs.h"
#include "mcmini/transitions/semaphore/MCSemaphoreDefs.h"
#include "mcmini/transitions/threads/MCThreadDefs.h"
#include <pthread.h>
#include <typeinfo>

extern "C" {
#include "mcmini/transitions/wrappers/MCSharedLibraryWrappers.h"
}

/* Source program thread control */
template<typename SharedMemoryData>
void
thread_post_visible_operation_hit(const std::type_info &type,
                                  SharedMemoryData *shmData)
{
  auto newTypeInfo = MCSharedTransition(tid_self, type);
  auto newShmData  = shmData;
  memcpy(shmTransitionTypeInfo, &newTypeInfo,
         sizeof(MCSharedTransition));
  memcpy(shmTransitionData, newShmData, sizeof(SharedMemoryData));
}

void thread_post_visible_operation_hit(const std::type_info &type);

void thread_await_scheduler();
void thread_await_scheduler_for_thread_start_transition();
void thread_awake_scheduler_for_thread_finish_transition();
void mc_exit_main_thread();

#endif // INCLUDE_MCMINI_TRANSITIONS_MCTRANSITIONSSHARED_HPP
