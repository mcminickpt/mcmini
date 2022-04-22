#ifndef GMAL_GMALTRANSITIONSSHARED_H
#define GMAL_GMALTRANSITIONSSHARED_H

#include <pthread.h>
#include <typeinfo>
#include "../GMALShared.h"

extern "C" {
    #include "GMALSharedLibraryWrappers.h"
}

#include "transitions/GMALThreadCreate.h"
#include "transitions/GMALThreadStart.h"
#include "transitions/GMALThreadFinish.h"
#include "transitions/GMALThreadJoin.h"
#include "transitions/GMALMutexInit.h"
#include "transitions/GMALMutexLock.h"
#include "transitions/GMALMutexUnlock.h"

/* Source program thread control */
template<typename SharedMemoryData> void
thread_post_visible_operation_hit(const std::type_info &type, SharedMemoryData * shmData);
void thread_post_visible_operation_hit(const std::type_info &type);

void thread_await_gmal_scheduler();
void thread_await_gmal_scheduler_for_thread_start_transition();
void thread_awake_gmal_scheduler_for_thread_finish_transition();
void gmal_exit_main_thread();


#endif //GMAL_GMALTRANSITIONSSHARED_H
