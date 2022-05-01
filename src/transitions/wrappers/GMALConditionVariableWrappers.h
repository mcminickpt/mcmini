#ifndef GMAL_GMALCONDITIONVARIABLEWRAPPERS_H
#define GMAL_GMALCONDITIONVARIABLEWRAPPERS_H

#include <pthread.h>
#include "GMALShared.h"

GMAL_EXTERN int gmal_pthread_cond_init(pthread_cond_t*, const pthread_condattr_t*);
GMAL_EXTERN int gmal_pthread_cond_wait(pthread_cond_t*, pthread_mutex_t*);
GMAL_EXTERN int gmal_pthread_cond_signal(pthread_cond_t*);
GMAL_EXTERN int gmal_pthread_cond_broadcast(pthread_cond_t*);

#endif //GMAL_GMALCONDITIONVARIABLEWRAPPERS_H
