#include "transitions/MCTransitionsShared.h"

// NOTE: Assumes that the parent process
// is asleep (called dpor_run_thread_to_next_visible_operation); the
// behavior is undefined otherwise
void
thread_await_scheduler()
{
  MC_ASSERT(tid_self != TID_INVALID);
  mc_shared_sem_ref cv = &(*trace_sleep_list)[tid_self];
  mc_shared_sem_wake_scheduler(cv);
  mc_shared_sem_wait_for_scheduler(cv);
}

// NOTE: This should only be called in one location:
// When the scheduler starts, there is an initial
// race condition between the child process and the
// parent process with `thread_await_dpor_scheduler`.
// `thread_await_dpor_scheduler` assumes the scheduler (parent)
// process is asleep; but upon initialization this is not true. Hence,
// this method is invoked instead
void
thread_await_scheduler_for_thread_start_transition()
{
  MC_ASSERT(tid_self != TID_INVALID);
  mc_shared_sem_ref cv = &(*trace_sleep_list)[tid_self];
  mc_shared_sem_wait_for_scheduler(cv);
}

void
thread_awake_scheduler_for_thread_finish_transition()
{
  MC_ASSERT(tid_self != TID_INVALID);
  mc_shared_sem_ref cv = &(*trace_sleep_list)[tid_self];
  mc_shared_sem_wake_scheduler(cv);
}

void
thread_post_visible_operation_hit(const std::type_info &type)
{
  auto newTypeInfo = MCSharedTransition(tid_self, type);
  // NOTE:  This cast could be done in a more complicated C++ way:
  //        https://stackoverflow.com/questions/66368061/error-clearing-an-object-of-non-trivial-type-with-memset
  memcpy((void *)shmTransitionTypeInfo, (void *)(&newTypeInfo),
         sizeof(MCSharedTransition));
}
