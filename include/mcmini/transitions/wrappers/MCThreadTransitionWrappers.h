#ifndef INCLUDE_MCMINI_TRANSITIONS_WRAPPERS_MCTHREADTRANSITIONWRAPPERS_HPP
#define INCLUDE_MCMINI_TRANSITIONS_WRAPPERS_MCTHREADTRANSITIONWRAPPERS_HPP

#include "mcmini/MCShared.h"
#include <pthread.h>

MC_EXTERN MC_NO_RETURN void mc_transparent_exit(int);
MC_EXTERN MC_NO_RETURN void mc_transparent_abort();
MC_EXTERN int mc_pthread_create(pthread_t *, const pthread_attr_t *,
                                void *(*)(void *), void *);
MC_EXTERN int mc_pthread_join(pthread_t, void **);

#endif // INCLUDE_MCMINI_TRANSITIONS_WRAPPERS_MCTHREADTRANSITIONWRAPPERS_HPP
