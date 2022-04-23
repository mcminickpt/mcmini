#ifndef GMAL_GMALTHREADTRANSITIONWRAPPERS_H
#define GMAL_GMALTHREADTRANSITIONWRAPPERS_H

#include <pthread.h>
#include <typeinfo>
#include "GMALShared.h"
#include "transitions/GMALTransitionsShared.h"

int gmal_pthread_create(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *);
int gmal_pthread_join(pthread_t, void**);

#endif //GMAL_GMALTHREADTRANSITIONWRAPPERS_H
