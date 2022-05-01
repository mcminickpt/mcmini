#ifndef GMAL_GMALBARRIERWRAPPERS_H
#define GMAL_GMALBARRIERWRAPPERS_H

#include <pthread.h>
#include "GMALShared.h"

GMAL_EXTERN int gmal_pthread_barrier_init(pthread_barrier_t*, const pthread_barrierattr_t*, unsigned int);
GMAL_EXTERN int gmal_pthread_barrier_wait(pthread_barrier_t *);


#endif //GMAL_GMALBARRIERWRAPPERS_H
