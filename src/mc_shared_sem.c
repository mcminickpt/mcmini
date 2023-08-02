#include "mcmini/mc_shared_sem.h"
#include "mcmini/transitions/wrappers/MCSharedLibraryWrappers.h"

// PRETTY_PRINT_DEF_DECL(mc_shared_sem)

void
mc_shared_sem_init(mc_shared_sem_ref ref)
{
  if (!ref) return;
  __real_sem_init(&ref->dpor_scheduler_sem, SEM_FLAG_SHARED, 0);
  __real_sem_init(&ref->pthread_sem, SEM_FLAG_SHARED, 0);
}

void
mc_shared_sem_destroy(mc_shared_sem_ref ref)
{
  if (!ref) return;

  // TODO: When we add sem_destroy, call __real_sem_destroy
  sem_destroy(&ref->dpor_scheduler_sem);
  sem_destroy(&ref->pthread_sem);
}

void
mc_shared_sem_wait_for_thread(mc_shared_sem_ref ref)
{
  __real_sem_wait(&ref->dpor_scheduler_sem);
}

void
mc_shared_sem_wait_for_scheduler(mc_shared_sem_ref ref)
{
  __real_sem_wait(&ref->pthread_sem);
}

void
mc_shared_sem_wake_thread(mc_shared_sem_ref ref)
{
  __real_sem_post(&ref->pthread_sem);
}

void
mc_shared_sem_wake_scheduler(mc_shared_sem_ref ref)
{
  __real_sem_post(&ref->dpor_scheduler_sem);
}
