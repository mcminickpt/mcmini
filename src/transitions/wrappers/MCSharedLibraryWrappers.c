#define _GNU_SOURCE
#include "mcmini/transitions/wrappers/MCSharedLibraryWrappers.h"
#include "mcmini/mcmini_wrappers.h"

typeof(&pthread_create) pthread_create_ptr;
typeof(&pthread_join) pthread_join_ptr;
typeof(&pthread_mutex_init) pthread_mutex_init_ptr;
typeof(&pthread_mutex_lock) pthread_mutex_lock_ptr;
typeof(&pthread_mutex_unlock) pthread_mutex_unlock_ptr;
typeof(&sem_wait) sem_wait_ptr;
typeof(&sem_post) sem_post_ptr;
typeof(&sem_init) sem_init_ptr;
typeof(&exit) exit_ptr;
typeof(&abort) abort_ptr;
typeof(&pthread_barrier_init) pthread_barrier_init_ptr;
typeof(&pthread_barrier_wait) pthread_barrier_wait_ptr;
typeof(&pthread_cond_init) pthread_cond_init_ptr;
typeof(&pthread_cond_wait) pthread_cond_wait_ptr;
typeof(&pthread_cond_signal) pthread_cond_signal_ptr;
typeof(&pthread_cond_broadcast) pthread_cond_broadcast_ptr;
typeof(&pthread_rwlock_init) pthread_rwlock_init_ptr;
typeof(&pthread_rwlock_rdlock) pthread_rwlock_rdlock_ptr;
typeof(&pthread_rwlock_wrlock) pthread_rwlock_wrlock_ptr;
typeof(&pthread_rwlock_unlock) pthread_rwlock_unlock_ptr;
typeof(&sleep) sleep_ptr;

void
mc_load_intercepted_symbol_addresses()
{
#if MC_SHARED_LIBRARY
  pthread_create_ptr       = dlsym(RTLD_NEXT, "pthread_create");
  pthread_join_ptr         = dlsym(RTLD_NEXT, "pthread_join");
  pthread_mutex_init_ptr   = dlsym(RTLD_NEXT, "pthread_mutex_init");
  pthread_mutex_lock_ptr   = dlsym(RTLD_NEXT, "pthread_mutex_lock");
  pthread_mutex_unlock_ptr = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
  sem_wait_ptr             = dlsym(RTLD_NEXT, "sem_wait");
  sem_post_ptr             = dlsym(RTLD_NEXT, "sem_post");
  sem_init_ptr             = dlsym(RTLD_NEXT, "sem_init");
  exit_ptr                 = dlsym(RTLD_NEXT, "exit");
  abort_ptr                = dlsym(RTLD_NEXT, "abort");
  pthread_barrier_init_ptr = dlsym(RTLD_NEXT, "pthread_barrier_init");
  pthread_barrier_wait_ptr = dlsym(RTLD_NEXT, "pthread_barrier_wait");
  pthread_rwlock_init_ptr  = dlsym(RTLD_NEXT, "pthread_rwlock_init");
  pthread_rwlock_rdlock_ptr =
    dlsym(RTLD_NEXT, "pthread_rwlock_rdlock");
  pthread_rwlock_wrlock_ptr =
    dlsym(RTLD_NEXT, "pthread_rwlock_wrlock");
  pthread_rwlock_unlock_ptr =
    dlsym(RTLD_NEXT, "pthread_rwlock_unlock");
  pthread_cond_init_ptr   = dlsym(RTLD_NEXT, "pthread_cond_init");
  pthread_cond_wait_ptr   = dlsym(RTLD_NEXT, "pthread_cond_wait");
  pthread_cond_signal_ptr = dlsym(RTLD_NEXT, "pthread_cond_signal");
  pthread_cond_broadcast_ptr =
    dlsym(RTLD_NEXT, "pthread_cond_broadcast");
  sleep_ptr = dlsym(RTLD_NEXT, "sleep");
#else
  pthread_create_ptr         = &pthread_create;
  pthread_join_ptr           = &pthread_join;
  pthread_mutex_init_ptr     = &pthread_mutex_init;
  pthread_mutex_lock_ptr     = &pthread_mutex_lock;
  pthread_mutex_unlock_ptr   = &pthread_mutex_unlock;
  sem_post_ptr               = &sem_post;
  sem_wait_ptr               = &sem_wait;
  sem_init_ptr               = &sem_init;
  exit_ptr                   = &exit;
  abort_ptr                  = &abort;
  pthread_barrier_init_ptr   = &pthread_barrier_init;
  pthread_barrier_wait_ptr   = &pthread_barrier_wait;
  pthread_rwlock_init_ptr    = &pthread_rwlock_init;
  pthread_rwlock_unlock_ptr  = &pthread_rwlock_unlock;
  pthread_rwlock_rdlock_ptr  = &pthread_rwlock_rdlock;
  pthread_rwlock_wrlock_ptr  = &pthread_rwlock_wrlock;
  pthread_cond_init_ptr      = &pthread_cond_init;
  pthread_cond_wait_ptr      = &pthread_cond_wait;
  pthread_cond_signal_ptr    = &pthread_cond_signal;
  pthread_cond_broadcast_ptr = &pthread_cond_broadcast;
  sleep_ptr                  = &sleep;
#endif
}

#if MC_SHARED_LIBRARY

int
pthread_create(pthread_t *pthread, const pthread_attr_t *attr,
               void *(*routine)(void *), void *arg)
{
  return mc_pthread_create(pthread, attr, routine, arg);
}

int
pthread_join(pthread_t pthread, void **result)
{
  return mc_pthread_join(pthread, result);
}

int
pthread_mutex_init(pthread_mutex_t *mutex,
                   const pthread_mutexattr_t *mutexattr)
{
  return mc_pthread_mutex_init(mutex, mutexattr);
}

int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
  return mc_pthread_mutex_lock(mutex);
}

int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  return mc_pthread_mutex_unlock(mutex);
}

int
sem_init(sem_t *sem, int pshared, unsigned int value)
{
  return mc_sem_init(sem, pshared, value);
}

int
sem_post(sem_t *sem)
{
  return mc_sem_post(sem);
}

int
sem_wait(sem_t *sem)
{
  return mc_sem_wait(sem);
}

void
exit(int status)
{
  mc_transparent_exit(status);
}

void
abort()
{
  mc_transparent_abort();
}

int
pthread_barrier_init(pthread_barrier_t *barrier,
                     const pthread_barrierattr_t *attr,
                     unsigned int count)
{
  return mc_pthread_barrier_init(barrier, attr, count);
}

int
pthread_barrier_wait(pthread_barrier_t *barrier)
{
  return mc_pthread_barrier_wait(barrier);
}

int
pthread_rwlock_init(pthread_rwlock_t *rwlock,
                    const pthread_rwlockattr_t *attr)
{
  return mc_pthread_rwlock_init(rwlock, attr);
}

int
pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
  return mc_pthread_rwlock_rdlock(rwlock);
}

int
pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
  return mc_pthread_rwlock_wrlock(rwlock);
}

int
pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
  return mc_pthread_rwlock_unlock(rwlock);
}

int
pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
  return mc_pthread_rwlock_destroy(rwlock);
}

int
pthread_cond_init(pthread_cond_t *cond,
                  const pthread_condattr_t *attr)
{
  return mc_pthread_cond_init(cond, attr);
}

int
pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  return mc_pthread_cond_wait(cond, mutex);
}

int
pthread_cond_signal(pthread_cond_t *cond)
{
  return mc_pthread_cond_signal(cond);
}

int
pthread_cond_broadcast(pthread_cond_t *cond)
{
  return mc_pthread_cond_broadcast(cond);
}

unsigned int
sleep(unsigned int seconds)
{
  /* Treat it as if no  time passed */
  return 0;
}

int
pthread_rwwlock_init(pthread_rwwlock_t *rwwlock)
{
  return mc_pthread_rwwlock_init(rwwlock);
}

int
pthread_rwwlock_rdlock(pthread_rwwlock_t *rwwlock)
{
  return mc_pthread_rwwlock_rdlock(rwwlock);
}

int
pthread_rwwlock_wr1lock(pthread_rwwlock_t *rwwlock)
{
  return mc_pthread_rwwlock_wr1lock(rwwlock);
}

int
pthread_rwwlock_wr2lock(pthread_rwwlock_t *rwwlock)
{
  return mc_pthread_rwwlock_wr2lock(rwwlock);
}

int
pthread_rwwlock_unlock(pthread_rwwlock_t *rwwlock)
{
  return mc_pthread_rwwlock_unlock(rwwlock);
}

int
pthread_rwwlock_destroy(pthread_rwwlock_t *rwwlock)
{
  return mc_pthread_rwwlock_destroy(rwwlock);
}
#endif