#ifndef MC_MCBARRIERWRAPPERS_H
#define MC_MCBARRIERWRAPPERS_H

#include "mcmini/MCShared.h"
#include <pthread.h>

MC_EXTERN int mc_pthread_barrier_init(pthread_barrier_t *,
                                      const pthread_barrierattr_t *,
                                      unsigned int);
MC_EXTERN int mc_pthread_barrier_wait(pthread_barrier_t *);

#endif // MC_MCBARRIERWRAPPERS_H
