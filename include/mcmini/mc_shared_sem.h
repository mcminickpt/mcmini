#ifndef INCLUDE_MCMINI_MC_SHARED_SEM_HPP
#define INCLUDE_MCMINI_MC_SHARED_SEM_HPP

#include "mcmini/MCShared.h"
#include <semaphore.h>

MC_STRUCT_DECL(mc_shared_sem)
struct mc_shared_sem {
  sem_t dpor_scheduler_sem;
  sem_t pthread_sem;
};

void mc_shared_sem_init(mc_shared_sem_ref);
void mc_shared_sem_destroy(mc_shared_sem_ref);

void mc_shared_sem_wait_for_thread(mc_shared_sem_ref);
void mc_shared_sem_wait_for_scheduler(mc_shared_sem_ref);
void mc_shared_sem_wake_thread(mc_shared_sem_ref);
void mc_shared_sem_wake_scheduler(mc_shared_sem_ref);

#endif // INCLUDE_MCMINI_MC_SHARED_SEM_HPP
