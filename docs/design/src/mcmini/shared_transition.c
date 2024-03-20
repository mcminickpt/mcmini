#include "shared_transition.h"
#include "shared_sem.h"

typedef uint64_t tid_t;
#define MC_THREAD_LOCAL thread_local
extern MC_THREAD_LOCAL tid_t tid_self;
#define TID_INVALID               (-1ul) // ULONG_MAX
MC_THREAD_LOCAL tid_t tid_self = TID_INVALID;

void
thread_await_scheduler()
{
  MC_ASSERT(tid_self != TID_INVALID);
  shared_sem_ref cv = &(*trace_sleep_list)[tid_self];
  mc_shared_sem_wake_scheduler(cv);
  mc_shared_sem_wait_for_scheduler(cv);
}


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
