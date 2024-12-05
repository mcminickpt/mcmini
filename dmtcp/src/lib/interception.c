#define _GNU_SOURCE
#include "mcmini/spy/intercept/interception.h"

#include <assert.h>
#include <dlfcn.h>
#include <dmtcp.h>
#include <stdbool.h>
#include <stdio.h>

#include "mcmini/spy/intercept/wrappers.h"

pthread_once_t libmcini_init = PTHREAD_ONCE_INIT;

typeof(&pthread_create) libpthread_pthread_create_ptr;
typeof(&pthread_create) libdmtcp_pthread_create_ptr;
typeof(&pthread_join) libpthread_pthread_join_ptr;
typeof(&pthread_join) libdmtcp_pthread_join_ptr;
typeof(&pthread_mutex_init) pthread_mutex_init_ptr;
typeof(&pthread_mutex_lock) pthread_mutex_lock_ptr;
typeof(&pthread_mutex_trylock) pthread_mutex_trylock_ptr;
typeof(&pthread_mutex_timedlock) pthread_mutex_timedlock_ptr;
typeof(&pthread_mutex_unlock) pthread_mutex_unlock_ptr;
typeof(&pthread_mutex_destroy) pthread_mutex_destroy_ptr;
typeof(&sem_wait) sem_wait_ptr;
typeof(&sem_post) sem_post_ptr;
typeof(&sem_init) sem_init_ptr;
typeof(&sem_destroy) sem_destroy_ptr;
typeof(&pthread_cond_init) pthread_cond_init_ptr;
typeof(&pthread_cond_wait) pthread_cond_wait_ptr;
typeof(&pthread_cond_timedwait) pthread_cond_timedwait_ptr;
typeof(&pthread_cond_signal) pthread_cond_signal_ptr;
typeof(&pthread_cond_broadcast) pthread_cond_broadcast_ptr;
typeof(&pthread_cond_destroy) pthread_cond_destroy_ptr;
typeof(&sleep) sleep_ptr;
__attribute__((__noreturn__)) typeof(&exit) exit_ptr;
__attribute__((__noreturn__)) typeof(&abort) abort_ptr;

void libmcmini_init(void) {
  pthread_once(&libmcini_init, &mc_load_intercepted_pthread_functions);
}

void mc_load_intercepted_pthread_functions(void) {
  void *libpthread_handle =
      dlopen("libpthread.so", RTLD_LAZY);

  if (!libpthread_handle) {
    libpthread_handle = dlopen("libpthread.so.0", RTLD_LAZY);
  }

  if (!libpthread_handle) {
    fprintf(stderr, "dlopen(3) couldn't find `libpthread`: %s\n", dlerror());
    fflush(stderr);
    libc_abort();
  }

  void *libc_handle = dlopen("libc.so", RTLD_LAZY);
  if (!libc_handle) {
    libc_handle = dlopen("libc.so.6", RTLD_LAZY);
  }

  if (!libc_handle) {
    fprintf(stderr, "dlopen(3) couldn't find `libc`: %s\n", dlerror());
    fflush(stderr);
    libc_abort();
  }

  libpthread_pthread_create_ptr = dlsym(libpthread_handle, "pthread_create");
  libpthread_pthread_join_ptr = dlsym(libpthread_handle, "pthread_join");
  pthread_mutex_init_ptr = dlsym(libpthread_handle, "pthread_mutex_init");
  pthread_mutex_lock_ptr = dlsym(libpthread_handle, "pthread_mutex_lock");
  pthread_mutex_trylock_ptr = dlsym(libpthread_handle, "pthread_mutex_trylock");
  pthread_mutex_timedlock_ptr = dlsym(libpthread_handle, "pthread_mutex_timedlock");
  pthread_mutex_unlock_ptr = dlsym(libpthread_handle, "pthread_mutex_unlock");
  pthread_mutex_destroy_ptr = dlsym(libpthread_handle, "pthread_mutex_destroy");
  sem_wait_ptr = dlsym(libpthread_handle, "sem_wait");
  sem_post_ptr = dlsym(libpthread_handle, "sem_post");
  sem_init_ptr = dlsym(libpthread_handle, "sem_init");
  pthread_cond_init_ptr = dlsym(libpthread_handle, "pthread_cond_init");
  pthread_cond_wait_ptr = dlsym(libpthread_handle, "pthread_cond_wait");
  pthread_cond_timedwait_ptr = dlsym(libpthread_handle, "pthread_cond_timedwait");
  pthread_cond_signal_ptr = dlsym(libpthread_handle, "pthread_cond_signal");
  pthread_cond_broadcast_ptr = dlsym(libpthread_handle, "pthread_cond_broadcast");
  pthread_cond_destroy_ptr = dlsym(libpthread_handle, "pthread_cond_destroy");
  sleep_ptr = dlsym(libc_handle, "sleep");
  exit_ptr = dlsym(libc_handle, "exit");
  abort_ptr = dlsym(libc_handle, "abort");

  dlclose(libpthread_handle);

  dlclose(libc_handle);

  if (dmtcp_is_enabled()) {
    // TODO: This shouldn't refer to the absolute path to dmtcp.so
    // We need to -->
    void *libdmtcp_handle = dlopen("libdmtcp.so", RTLD_LAZY);
    if (!libdmtcp_handle) {
      fprintf(stderr, "dlopen(3) couldn't find `libdmtcp`: %s\n", dlerror());
      fflush(stderr);
      libc_abort();
    }
    libdmtcp_pthread_create_ptr = dlsym(libdmtcp_handle, "pthread_create");
    libdmtcp_pthread_join_ptr = dlsym(libdmtcp_handle, "pthread_join");
    dlclose(libdmtcp_handle);
  }
}

int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *mutexattr) {
  return mc_pthread_mutex_init(mutex, mutexattr);
}

int libpthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *attr) {
  libmcmini_init();
  return (*pthread_mutex_init_ptr)(mutex, attr);
}

int pthread_mutex_lock(pthread_mutex_t *mutex) {
  return mc_pthread_mutex_lock(mutex);
}

int libpthread_mutex_lock(pthread_mutex_t *mut) {
  libmcmini_init();
  return (*pthread_mutex_lock_ptr)(mut);
}

int libpthread_mutex_trylock(pthread_mutex_t *mut) {
  libmcmini_init();
  return (*pthread_mutex_trylock_ptr)(mut);
}

int libpthread_mutex_timedlock(pthread_mutex_t *mut, struct timespec *t) {
  libmcmini_init();
  return (*pthread_mutex_timedlock_ptr)(mut, t);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {
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

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
  return mc_pthread_cond_init(cond, attr);
}

int libpthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
  libmcmini_init();
  return (*pthread_cond_init_ptr)(cond, attr);
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mut) {
  return mc_pthread_cond_wait(cond, mut);
}

int libpthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mut) {
  libmcmini_init();
  return (*pthread_cond_wait_ptr)(cond, mut);
}

int libpthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mut,
                              const struct timespec *abstime) {
  libmcmini_init();
  return (*pthread_cond_timedwait_ptr)(cond, mut, abstime);
}

int pthread_cond_signal(pthread_cond_t *cond) {
  return mc_pthread_cond_signal(cond);
}

int libpthread_cond_signal(pthread_cond_t *cond) {
  libmcmini_init();
  return (*pthread_cond_signal_ptr)(cond);
}

int pthread_cond_broadcast(pthread_cond_t *cond) {
  return mc_pthread_cond_broadcast(cond);
}

int libpthread_cond_broadcast(pthread_cond_t *cond) {
  libmcmini_init();
  return (*pthread_cond_broadcast_ptr)(cond);
}

int pthread_cond_destroy(pthread_cond_t *cond) {
  return mc_pthread_cond_destroy(cond);
}

int libpthread_cond_destroy(pthread_cond_t *cond) {
  libmcmini_init();
  return (*pthread_cond_destroy_ptr)(cond);
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*routine)(void *), void *arg) {
  return mc_pthread_create(thread, attr, routine, arg);
}

int libpthread_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                              void *(*routine)(void *), void *arg) {
  libmcmini_init();
  return (*libpthread_pthread_create_ptr)(thread, attr, routine, arg);
}

int libdmtcp_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                            void *(*routine)(void *), void *arg) {
  libmcmini_init();
  return (*libdmtcp_pthread_create_ptr)(thread, attr, routine, arg);
}

int pthread_join(pthread_t thread, void **rv) {
  return mc_pthread_join(thread, rv);
}
int libpthread_pthread_join(pthread_t thread, void **rv) {
  libmcmini_init();
  return (*libpthread_pthread_join_ptr)(thread, rv);
}
int libdmtcp_pthread_join(pthread_t thread, void **rv) {
  libmcmini_init();
  return (*libdmtcp_pthread_join_ptr)(thread, rv);
}

void exit(int status) {
  mc_transparent_exit(status);
}

void abort(void) {
  mc_transparent_abort();
}

unsigned sleep(unsigned duration) {
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
