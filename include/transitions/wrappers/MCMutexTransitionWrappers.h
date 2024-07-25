#ifndef MC_MCMUTEXTRANSITIONWRAPPERS_H
#define MC_MCMUTEXTRANSITIONWRAPPERS_H

#include "mcmini/MCShared.h"
#include <pthread.h>

MC_EXTERN int mc_pthread_mutex_init(pthread_mutex_t *,
                                    const pthread_mutexattr_t *);
MC_EXTERN int mc_pthread_mutex_lock(pthread_mutex_t *);
MC_EXTERN int mc_pthread_mutex_unlock(pthread_mutex_t *);

#endif // MC_MCMUTEXTRANSITIONWRAPPERS_H
