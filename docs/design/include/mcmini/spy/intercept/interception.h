#pragma once

#define _GNU_SOURCE
#include <pthread.h>

int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr);

int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *mutexattr) {
  return mc_pthread_mutex_init(mutex, mutexattr);
}

// TODO: Other wrappers here