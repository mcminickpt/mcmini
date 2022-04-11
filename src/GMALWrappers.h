#ifndef GMAL_GMALWRAPPERS_H
#define GMAL_GMALWRAPPERS_H

#include <pthread.h>

int gmal_pthread_create(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *);
int gmal_pthread_join(pthread_t, void**);

#endif //GMAL_GMALWRAPPERS_H
