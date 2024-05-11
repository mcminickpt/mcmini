#pragma once

#define _GNU_SOURCE

#include "mcmini/defines.h"
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

void mc_load_intercepted_pthread_functions();

int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *mutexattr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);


void exit(int);
MCMINI_NO_RETURN void libc_exit(int);

void abort(void);
MCMINI_NO_RETURN void libc_abort(void);