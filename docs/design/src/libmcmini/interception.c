#define _GNU_SOURCE
#include "mcmini/spy/intercept/interception.h"

#include <dlfcn.h>

#include "mcmini/spy/intercept/wrappers.h"

typeof(&pthread_create) pthread_create_ptr;
typeof(&pthread_join) pthread_join_ptr;
typeof(&pthread_mutex_init) pthread_mutex_init_ptr;
typeof(&pthread_mutex_lock) pthread_mutex_lock_ptr;
typeof(&pthread_mutex_unlock) pthread_mutex_unlock_ptr;
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

void mc_load_intercepted_pthread_functions() {
  pthread_create_ptr = dlsym(RTLD_NEXT, "pthread_create");
  pthread_join_ptr = dlsym(RTLD_NEXT, "pthread_join");
  pthread_mutex_init_ptr = dlsym(RTLD_NEXT, "pthread_mutex_init");
  pthread_mutex_lock_ptr = dlsym(RTLD_NEXT, "pthread_mutex_lock");
  pthread_mutex_unlock_ptr = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
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
  return mc_pthread_mutex_init(mutex, mutexattr);
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
  return mc_pthread_mutex_lock(mutex);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
  return mc_pthread_mutex_unlock(mutex);
}