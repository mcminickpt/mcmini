#ifndef GMAL_GMALTHREADTRANSITIONWRAPPERS_H
#define GMAL_GMALTHREADTRANSITIONWRAPPERS_H

#include <pthread.h>
#include "GMALShared.h"

GMAL_EXTERN GMAL_NO_RETURN void gmal_exit(int);
GMAL_EXTERN void gmal_pthread_reach_goal();
GMAL_EXTERN int gmal_pthread_create(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *);
GMAL_EXTERN int gmal_pthread_join(pthread_t, void**);

#define GOAL() gmal_pthread_reach_goal()

#endif //GMAL_GMALTHREADTRANSITIONWRAPPERS_H
