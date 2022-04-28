#ifndef GMAL_GMALTHREADTRANSITIONWRAPPERS_H
#define GMAL_GMALTHREADTRANSITIONWRAPPERS_H

#include <pthread.h>
#include "GMALShared.h"

GMAL_EXTERN int gmal_pthread_create(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *);
GMAL_EXTERN int gmal_pthread_join(pthread_t, void**);

#endif //GMAL_GMALTHREADTRANSITIONWRAPPERS_H
