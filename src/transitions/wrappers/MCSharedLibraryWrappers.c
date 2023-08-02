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

int mcmini_enabled = -1; // -1 indicates uninitialized state

// Function to check the environment variable and initialize mcmini_enabled
void check_mcmini_enabled() {
    if (mcmini_enabled == -1) {
        const char* env_enabled = getenv("MCMINI_ENABLED");
        if (env_enabled && strcmp(env_enabled, "0") == 0) mcmini_enabled=0;
    }
}


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
  if(!mcmini_enabled) return mc_pthread_create(pthread, attr, routine, arg);
  else pthread_create_ptr = dlsym(RTLD_NEXT, "pthread_create");
}

int
pthread_join(pthread_t pthread, void **result)
{
  if(!mcmini_enabled) return mc_pthread_join(pthread, result);
  else pthread_join_ptr         = dlsym(RTLD_NEXT, "pthread_join");
}

int
pthread_mutex_init(pthread_mutex_t *mutex,
                   const pthread_mutexattr_t *mutexattr)
{
  if(!mcmini_enabled) return mc_pthread_mutex_init(mutex, mutexattr);
  else pthread_mutex_init_ptr   = dlsym(RTLD_NEXT, "pthread_mutex_init");
}

int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
  if(!mcmini_enabled) mc_pthread_mutex_lock(mutex);
  else pthread_mutex_lock_ptr   = dlsym(RTLD_NEXT, "pthread_mutex_lock");
}

int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  if(!mcmini_enabled) return mc_pthread_mutex_unlock(mutex);
  else pthread_mutex_unlock_ptr = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
}

int
sem_init(sem_t *sem, int pshared, unsigned int value)
{
  if(!mcmini_enabled) return mc_sem_init(sem, pshared, value);
  else sem_init_ptr             = dlsym(RTLD_NEXT, "sem_init");
}

int
sem_post(sem_t *sem)
{
  if(!mcmini_enabled) return mc_sem_post(sem);
  else sem_post_ptr             = dlsym(RTLD_NEXT, "sem_post");
}

int
sem_wait(sem_t *sem)
{
  if(!mcmini_enabled) return mc_sem_wait(sem);
  else sem_wait_ptr             = dlsym(RTLD_NEXT, "sem_wait");
}

void
exit(int status)
{
  if(!mcmini_enabled) mc_transparent_exit(status);
  else exit_ptr                 = dlsym(RTLD_NEXT, "exit");
}

void
abort()
{
  if(mcmini_enabled) mc_transparent_abort();
  else abort_ptr                = dlsym(RTLD_NEXT, "abort");
}

int
pthread_barrier_init(pthread_barrier_t *barrier,
                     const pthread_barrierattr_t *attr,
                     unsigned int count)
{
  if(!mcmini_enabled) return mc_pthread_barrier_init(barrier, attr, count);
  pthread_barrier_init_ptr = dlsym(RTLD_NEXT, "pthread_barrier_init");
}

int
pthread_barrier_wait(pthread_barrier_t *barrier)
{
  if(!mcmini_enabled) return mc_pthread_barrier_wait(barrier);
  else pthread_barrier_wait_ptr = dlsym(RTLD_NEXT, "pthread_barrier_wait");
}

int
pthread_rwlock_init(pthread_rwlock_t *rwlock,
                    const pthread_rwlockattr_t *attr)
{
  if(!mcmini_enabled) return mc_pthread_rwlock_init(rwlock, attr);
  else pthread_rwlock_init_ptr  = dlsym(RTLD_NEXT, "pthread_rwlock_init");
}

int
pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
  if(!mcmini_enabled) return mc_pthread_rwlock_rdlock(rwlock);
  else pthread_rwlock_rdlock_ptr = dlsym(RTLD_NEXT, "pthread_rwlock_rdlock");
}

int
pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
  if(!mcmini_enabled) return mc_pthread_rwlock_wrlock(rwlock);
  else   pthread_rwlock_wrlock_ptr = dlsym(RTLD_NEXT, "pthread_rwlock_wrlock");
}

int
pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
  if(!mcmini_enabled) return mc_pthread_rwlock_unlock(rwlock);
  else  pthread_rwlock_unlock_ptr = dlsym(RTLD_NEXT, "pthread_rwlock_unlock");
}

// int
// pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
// {
//   if(!mcmini_enabled) return mc_pthread_rwlock_destroy(rwlock);
//   else typeof(&pthread_rwlock_destroy) pthread_rwlock_destroy_ptr = dlsym(RTLD_NEXT, "pthread_rwlock_destroy");
// }

int
pthread_cond_init(pthread_cond_t *cond,
                  const pthread_condattr_t *attr)
{
  if(!mcmini_enabled) return mc_pthread_cond_init(cond, attr);
  else pthread_cond_init_ptr   = dlsym(RTLD_NEXT, "pthread_cond_init");
}

int
pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  if(!mcmini_enabled) return mc_pthread_cond_wait(cond, mutex);
  else pthread_cond_wait_ptr   = dlsym(RTLD_NEXT, "pthread_cond_wait");
}

int
pthread_cond_signal(pthread_cond_t *cond)
{
  if(!mcmini_enabled) return mc_pthread_cond_signal(cond);
  else pthread_cond_signal_ptr = dlsym(RTLD_NEXT, "pthread_cond_signal");
}

int
pthread_cond_broadcast(pthread_cond_t *cond)
{
  if(!mcmini_enabled) return mc_pthread_cond_broadcast(cond);
  else   pthread_cond_broadcast_ptr = dlsym(RTLD_NEXT, "pthread_cond_broadcast");
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
  if(!mcmini_enabled) return mc_pthread_rwwlock_init(rwwlock);
}

int
pthread_rwwlock_rdlock(pthread_rwwlock_t *rwwlock)
{
  if(!mcmini_enabled) return mc_pthread_rwwlock_rdlock(rwwlock);
}

int
pthread_rwwlock_wr1lock(pthread_rwwlock_t *rwwlock)
{
  if(!mcmini_enabled) return mc_pthread_rwwlock_wr1lock(rwwlock);
}

int
pthread_rwwlock_wr2lock(pthread_rwwlock_t *rwwlock)
{
  if(!mcmini_enabled) return mc_pthread_rwwlock_wr2lock(rwwlock);
}

int
pthread_rwwlock_unlock(pthread_rwwlock_t *rwwlock)
{
  if(!mcmini_enabled) return mc_pthread_rwwlock_unlock(rwwlock);
}

int
pthread_rwwlock_destroy(pthread_rwwlock_t *rwwlock)
{
  if(!mcmini_enabled) return mc_pthread_rwwlock_destroy(rwwlock);
}
#endif