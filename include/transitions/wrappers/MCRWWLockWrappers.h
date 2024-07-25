#ifndef INCLUDE_MCMINI_TRANSITIONS_WRAPPERS_MCRWWLOCKWRAPPERS_HPP
#define INCLUDE_MCMINI_TRANSITIONS_WRAPPERS_MCRWWLOCKWRAPPERS_HPP

#include "mcmini/MCShared.h"
#include "mcmini/export/rwwlock.h"

MC_EXTERN int mc_pthread_rwwlock_init(pthread_rwwlock_t *);
MC_EXTERN int mc_pthread_rwwlock_rdlock(pthread_rwwlock_t *);
MC_EXTERN int mc_pthread_rwwlock_wr1lock(pthread_rwwlock_t *);
MC_EXTERN int mc_pthread_rwwlock_wr2lock(pthread_rwwlock_t *);
MC_EXTERN int mc_pthread_rwwlock_unlock(pthread_rwwlock_t *);
MC_EXTERN int mc_pthread_rwwlock_destroy(pthread_rwwlock_t *);

#endif // INCLUDE_MCMINI_TRANSITIONS_WRAPPERS_MCRWWLOCKWRAPPERS_HPP
