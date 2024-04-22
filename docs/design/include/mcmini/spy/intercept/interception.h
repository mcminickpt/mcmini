#pragma once

#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

void mc_load_intercepted_pthread_functions();

int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *mutexattr);

// TODO: Other wrappers here