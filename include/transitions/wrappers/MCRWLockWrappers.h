#ifndef INCLUDE_MCMINI_TRANSITIONS_WRAPPERS_MCRWLOCKWRAPPERS_HPP
#define INCLUDE_MCMINI_TRANSITIONS_WRAPPERS_MCRWLOCKWRAPPERS_HPP

#include "mcmini/MCShared.h"
#include <pthread.h>

MC_EXTERN int mc_pthread_rwlock_init(pthread_rwlock_t *,
                                     const pthread_rwlockattr_t *);
MC_EXTERN int mc_pthread_rwlock_rdlock(pthread_rwlock_t *);
MC_EXTERN int mc_pthread_rwlock_wrlock(pthread_rwlock_t *);
MC_EXTERN int mc_pthread_rwlock_unlock(pthread_rwlock_t *);
MC_EXTERN int mc_pthread_rwlock_destroy(pthread_rwlock_t *);

#endif // INCLUDE_MCMINI_TRANSITIONS_WRAPPERS_MCRWLOCKWRAPPERS_HPP
