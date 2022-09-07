#ifndef MC_MCSHAREDLIBRARYWRAPPERS_H
#define MC_MCSHAREDLIBRARYWRAPPERS_H

#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

extern typeof(&pthread_create) pthread_create_ptr;
extern typeof(&pthread_join) pthread_join_ptr;
extern typeof(&pthread_mutex_init) pthread_mutex_init_ptr;
extern typeof(&pthread_mutex_lock) pthread_mutex_lock_ptr;
extern typeof(&pthread_mutex_unlock) pthread_mutex_unlock_ptr;
extern typeof(&sem_wait) sem_wait_ptr;
extern typeof(&sem_post) sem_post_ptr;
extern typeof(&sem_init) sem_init_ptr;
extern typeof(&exit) exit_ptr;
extern typeof(&pthread_barrier_init) pthread_barrier_init_ptr;
extern typeof(&pthread_barrier_wait) pthread_barrier_wait_ptr;
extern typeof(&pthread_cond_init) pthread_cond_init_ptr;
extern typeof(&pthread_cond_wait) pthread_cond_wait_ptr;
extern typeof(&pthread_cond_signal) pthread_cond_signal_ptr;
extern typeof(&pthread_cond_broadcast) pthread_cond_broadcast_ptr;
extern typeof(&pthread_rwlock_init) pthread_rwlock_init_ptr;
extern typeof(&pthread_rwlock_rdlock) pthread_rwlock_rdlock_ptr;
extern typeof(&pthread_rwlock_wrlock) pthread_rwlock_wrlock_ptr;
extern typeof(&pthread_rwlock_unlock) pthread_rwlock_unlock_ptr;
extern typeof(&sleep) sleep_ptr;

#define __real_pthread_create         (*pthread_create_ptr)
#define __real_pthread_join           (*pthread_join_ptr)
#define __real_pthread_mutex_init     (*pthread_mutex_init_ptr)
#define __real_pthread_mutex_lock     (*pthread_mutex_lock_ptr)
#define __real_pthread_mutex_unlock   (*pthread_mutex_unlock_ptr)
#define __real_sem_wait               (*sem_wait_ptr)
#define __real_sem_post               (*sem_post_ptr)
#define __real_sem_init               (*sem_init_ptr)
#define __real_exit                   (*exit_ptr)
#define __real_pthread_barrier_init   (*pthread_barrier_init_ptr)
#define __real_pthread_barrier_wait   (*pthread_barrier_wait_ptr)
#define __real_pthread_cond_init      (*pthread_cond_init_ptr)
#define __real_pthread_cond_wait      (*pthread_cond_wait_ptr)
#define __real_pthread_cond_signal    (*pthread_cond_signal_ptr)
#define __real_pthread_cond_broadcast (*pthread_cond_broadcast_ptr)
#define __real_pthread_rwlock_init    (*pthread_rwlock_init_ptr)
#define __real_pthread_rwlock_rdlock  (*pthread_rwlock_rdlock_ptr)
#define __real_pthread_rwlock_wrlock  (*pthread_rwlock_wrlock_ptr)
#define __real_pthread_rwlock_unlock  (*pthread_rwlock_unlock_ptr)
#define __real_sleep                  (*sleep_ptr)

void mc_load_shadow_routines();

#endif // MC_MCSHAREDLIBRARYWRAPPERS_H
