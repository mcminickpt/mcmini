#ifndef DPOR_PTHREAD_WRAPPERS_H
#define DPOR_PTHREAD_WRAPPERS_H

#include <pthread.h>
#include "dpor.h"

int dpor_pthread_mutex_init(pthread_mutex_t*, pthread_mutexattr_t*);
int dpor_pthread_mutex_lock(pthread_mutex_t*);
int dpor_pthread_mutex_unlock(pthread_mutex_t*);
int dpor_pthread_mutex_lock(pthread_mutex_t*);
int dpor_pthread_mutex_destroy(pthread_mutex_t*);

int dpor_pthread_create(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *);
int dpor_pthread_join(pthread_t *, void**);

#endif //DPOR_PTHREAD_WRAPPERS_H
