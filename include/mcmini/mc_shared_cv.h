#ifndef DPOR_MC_SHARED_CV_H
#define DPOR_MC_SHARED_CV_H

#include "mcmini/MCShared.h"
#include <semaphore.h>

MC_STRUCT_DECL(mc_shared_cv)
struct mc_shared_cv {
  sem_t dpor_scheduler_sem;
  sem_t pthread_sem;
};

void mc_shared_cv_init(mc_shared_cv_ref);
void mc_shared_cv_destroy(mc_shared_cv_ref);

void mc_shared_cv_wait_for_thread(mc_shared_cv_ref);
void mc_shared_cv_wait_for_scheduler(mc_shared_cv_ref);
void mc_shared_cv_wake_thread(mc_shared_cv_ref);
void mc_shared_cv_wake_scheduler(mc_shared_cv_ref);

#endif // DPOR_MC_SHARED_CV_H
