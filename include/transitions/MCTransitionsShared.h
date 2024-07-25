#ifndef INCLUDE_MCMINI_TRANSITIONS_MCTRANSITIONSSHARED_HPP
#define INCLUDE_MCMINI_TRANSITIONS_MCTRANSITIONSSHARED_HPP

#include "MCShared.h"
#include "mcmini_private.h"
#include "transitions/barrier/MCBarrierDefs.h"
#include "transitions/cond/MCCondDefs.h"
#include "transitions/misc/MCMiscDefs.h"
#include "transitions/mutex/MCMutexDefs.h"
#include "transitions/rwlock/MCRWLockDefs.h"
#include "transitions/rwwlock/MCRWWLockDefs.h"
#include "transitions/semaphore/MCSemaphoreDefs.h"
#include "transitions/threads/MCThreadDefs.h"
#include <pthread.h>
#include <typeinfo>

extern "C" {
#include "transitions/wrappers/MCSharedLibraryWrappers.h"
}

/* Source program thread control */
template<typename SharedMemoryData>
void
thread_post_visible_operation_hit(const std::type_info &type,
                                  SharedMemoryData *shmData)
{
  auto newTypeInfo = MCSharedTransition(tid_self, type);
  auto newShmData  = shmData;
  // NOTE: This cast could also be done in a more complicated way for C++:
  //   https://stackoverflow.com/questions/57721104/avoid-wclass-memaccess-on-memcpy-of-a-pod-type-w-copy-disabled
  memcpy((char *)shmTransitionTypeInfo,
         (char *)(&newTypeInfo),
         sizeof(MCSharedTransition));
  memcpy((char *)shmTransitionData, (char *)newShmData,
         sizeof(SharedMemoryData));
}

void thread_post_visible_operation_hit(const std::type_info &type);

void thread_await_scheduler();
void thread_await_scheduler_for_thread_start_transition();
void thread_awake_scheduler_for_thread_finish_transition();
void mc_exit_main_thread();

#endif // INCLUDE_MCMINI_TRANSITIONS_MCTRANSITIONSSHARED_HPP
