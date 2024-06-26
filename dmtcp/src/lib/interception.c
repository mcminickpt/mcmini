#define _GNU_SOURCE
#include "mcmini/spy/intercept/interception.h"

#include <dlfcn.h>
#include <stdbool.h>

#include "mcmini/spy/intercept/wrappers.h"

bool libmcmini_initialized = false;

typeof(&pthread_create) pthread_create_ptr;
typeof(&pthread_join) pthread_join_ptr;
typeof(&pthread_mutex_init) pthread_mutex_init_ptr;
typeof(&pthread_mutex_lock) pthread_mutex_lock_ptr;
typeof(&pthread_mutex_unlock) pthread_mutex_unlock_ptr;
typeof(&pthread_mutex_destroy) pthread_mutex_destroy_ptr;
typeof(&sem_wait) sem_wait_ptr;
typeof(&sem_post) sem_post_ptr;
typeof(&sem_init) sem_init_ptr;
typeof(&sem_destroy) sem_destroy_ptr;
typeof(&pthread_cond_init) pthread_cond_init_ptr;
typeof(&pthread_cond_wait) pthread_cond_wait_ptr;
typeof(&pthread_cond_signal) pthread_cond_signal_ptr;
typeof(&pthread_cond_broadcast) pthread_cond_broadcast_ptr;
typeof(&sleep) sleep_ptr;
__attribute__((__noreturn__)) typeof(&exit) exit_ptr;
__attribute__((__noreturn__)) typeof(&abort) abort_ptr;

inline static void libmcmini_init() {
  if (!libmcmini_initialized) {
    mc_load_intercepted_pthread_functions();
    libmcmini_initialized = true;
  }
}

void mc_load_intercepted_pthread_functions() {
  pthread_create_ptr = dlsym(RTLD_NEXT, "pthread_create");
  pthread_join_ptr = dlsym(RTLD_NEXT, "pthread_join");
  pthread_mutex_init_ptr = dlsym(RTLD_NEXT, "pthread_mutex_init");
  pthread_mutex_lock_ptr = dlsym(RTLD_NEXT, "pthread_mutex_lock");
  pthread_mutex_unlock_ptr = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
  pthread_mutex_destroy_ptr = dlsym(RTLD_NEXT, "pthread_mutex_destroy");
  sem_wait_ptr = dlsym(RTLD_NEXT, "sem_wait");
  sem_post_ptr = dlsym(RTLD_NEXT, "sem_post");
  sem_init_ptr = dlsym(RTLD_NEXT, "sem_init");
  pthread_cond_init_ptr = dlsym(RTLD_NEXT, "pthread_cond_init");
  pthread_cond_wait_ptr = dlsym(RTLD_NEXT, "pthread_cond_wait");
  pthread_cond_signal_ptr = dlsym(RTLD_NEXT, "pthread_cond_signal");
  pthread_cond_broadcast_ptr = dlsym(RTLD_NEXT, "pthread_cond_broadcast");
  sleep_ptr = dlsym(RTLD_NEXT, "sleep");
  exit_ptr = dlsym(RTLD_NEXT, "exit");
  abort_ptr = dlsym(RTLD_NEXT, "abort");
}

int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *mutexattr) {
  libmcmini_init();
  return mc_pthread_mutex_init(mutex, mutexattr);
}

int libpthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *attr) {
  libmcmini_init();
  return (*pthread_mutex_init_ptr)(mutex, attr);
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
  libmcmini_init();
  return mc_pthread_mutex_lock(mutex);
}

int libpthread_mutex_lock(pthread_mutex_t *mut) {
  libmcmini_init();
  return (*pthread_mutex_lock_ptr)(mut);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
  libmcmini_init();
  return mc_pthread_mutex_unlock(mutex);
}

int libpthread_mutex_unlock(pthread_mutex_t *mut) {
  libmcmini_init();
  return (*pthread_mutex_unlock_ptr)(mut);
}

int libpthread_mutex_destroy(pthread_mutex_t *mut) {
  libmcmini_init();
  return (*pthread_mutex_destroy_ptr)(mut);
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*routine)(void *), void *arg) {
  libmcmini_init();
  return mc_pthread_create(thread, attr, routine, arg);
}

int pthread_join(pthread_t thread, void **rv) {
  libmcmini_init();
  return mc_pthread_join(thread, rv);
}
int libpthread_pthread_join(pthread_t thread, void **rv) {
  libmcmini_init();
  return (*pthread_join_ptr)(thread, rv);
}

void exit(int status) {
  libmcmini_init();
  mc_transparent_exit(status);
}

void abort(void) {
  libmcmini_init();
  mc_transparent_abort();
}

unsigned sleep(unsigned duration) {
  libmcmini_init();
  return mc_sleep(duration);
}

unsigned libc_sleep(unsigned duration) {
  libmcmini_init();
  return (*sleep_ptr)(duration);
}

// Forwarding methods to the underlying libraries
MCMINI_NO_RETURN void libc_abort(void) {
  libmcmini_init();
  (*abort_ptr)();
}
MCMINI_NO_RETURN void libc_exit(int status) {
  libmcmini_init();
  (*exit_ptr)(status);
}

int libpthread_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                              void *(*routine)(void *), void *arg) {
  libmcmini_init();
  return (*pthread_create_ptr)(thread, attr, routine, arg);
}
int libpthread_sem_init(sem_t *sem, int pshared, int value) {
  libmcmini_init();
  return (*sem_init_ptr)(sem, pshared, value);
}
int libpthread_sem_post(sem_t *sem) {
  libmcmini_init();
  return (*sem_post_ptr)(sem);
}
int libpthread_sem_wait(sem_t *sem) {
  libmcmini_init();
  return (*sem_wait_ptr)(sem);
}
