#ifndef MC_MCCONDITIONVARIABLEWRAPPERS_H
#define MC_MCCONDITIONVARIABLEWRAPPERS_H

#include "mcmini/MCShared.h"
#include <pthread.h>

MC_EXTERN int mc_pthread_cond_init(pthread_cond_t *,
                                   const pthread_condattr_t *);
MC_EXTERN int mc_pthread_cond_wait(pthread_cond_t *,
                                   pthread_mutex_t *);
MC_EXTERN int mc_pthread_cond_signal(pthread_cond_t *);
MC_EXTERN int mc_pthread_cond_broadcast(pthread_cond_t *);

#endif // MC_MCCONDITIONVARIABLEWRAPPERS_H
