#pragma once

#define _GNU_SOURCE

#include "mcmini/defines.h"
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

void libmcmini_init(void);
void mc_load_intercepted_pthread_functions(void);

int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *mutexattr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                  void *(*routine)(void *), void *arg);
int libpthread_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                  void *(*routine)(void *), void *arg);
int libdmtcp_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                            void *(*routine)(void *), void *arg);

int pthread_join(pthread_t thread, void**);
int libpthread_pthread_join(pthread_t thread, void**);
int libdmtcp_pthread_join(pthread_t thread, void**);

int libpthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *);
int libpthread_mutex_lock(pthread_mutex_t *);
int libpthread_mutex_trylock(pthread_mutex_t *);
int libpthread_mutex_timedlock(pthread_mutex_t *, struct timespec *);
int libpthread_mutex_unlock(pthread_mutex_t *);
int libpthread_mutex_destroy(pthread_mutex_t *);


int sem_init(sem_t*, int, unsigned);
int sem_post(sem_t*);
int sem_wait(sem_t*);
int sem_destroy(sem_t *);
int libpthread_sem_init(sem_t*, int, int);
int libpthread_sem_post(sem_t*);
int libpthread_sem_wait(sem_t*);
int libpthread_sem_destroy(sem_t *);
int libpthread_sem_timedwait(sem_t*, struct timespec *);

unsigned sleep(unsigned);
unsigned libc_sleep(unsigned);

void exit(int);
MCMINI_NO_RETURN void libc_exit(int);

void abort(void);
MCMINI_NO_RETURN void libc_abort(void);

pid_t libc_fork(void);
