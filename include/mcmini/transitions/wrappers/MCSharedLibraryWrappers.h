#ifndef INCLUDE_MCMINI_TRANSITIONS_WRAPPERS_MCSHAREDLIBRARYWRAPPERS_HPP
#define INCLUDE_MCMINI_TRANSITIONS_WRAPPERS_MCSHAREDLIBRARYWRAPPERS_HPP

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
extern __attribute__((__noreturn__)) typeof(&exit) exit_ptr;
extern __attribute__((__noreturn__)) typeof(&abort) abort_ptr;
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
#define __real_abort                  (*abort_ptr)
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

/**
 * @brief Retrieves the addresses of the symbols exposed by the
 * dynamic libraries that McMini redefines to override the
 * functions' original behavior
 *
 * McMini re-defines and exports symbols defined in other dynamic
 * libraries to change the behavior of a program which is dynamically
 * linked to the libraries containing those symbols. When McMini as a
 * dynamic libarary is pre-loaded into memory, a process making calls
 * to the dynamic library will result in McMini's symbols being
 * dynamically chosen instead.
 *
 * If the original behavior of the intercepted symbols is still
 * needed, they must be retrieved using dlsym(3) and other functions.
 * This function loads those symbols into the variables redefined
 * above.
 */
void mc_load_intercepted_symbol_addresses();

#endif // INCLUDE_MCMINI_TRANSITIONS_WRAPPERS_MCSHAREDLIBRARYWRAPPERS_HPP
