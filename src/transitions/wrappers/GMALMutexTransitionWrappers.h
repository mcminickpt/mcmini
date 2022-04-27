#ifndef GMAL_GMALMUTEXTRANSITIONWRAPPERS_H
#define GMAL_GMALMUTEXTRANSITIONWRAPPERS_H

#include <pthread.h>
#include "GMALShared.h"

GMAL_EXTERN int gmal_pthread_mutex_init(pthread_mutex_t*, const pthread_mutexattr_t*);
GMAL_EXTERN int gmal_pthread_mutex_lock(pthread_mutex_t*);
GMAL_EXTERN int gmal_pthread_mutex_unlock(pthread_mutex_t*);

#endif //GMAL_GMALMUTEXTRANSITIONWRAPPERS_H
