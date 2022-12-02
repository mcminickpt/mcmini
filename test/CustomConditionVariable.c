#include "mcmini/CustomConditionVariable.h"

int
custom_cond_init(custom_cond *cond)
{
  cond->numWaiters = 0;
  pthread_mutex_init(&cond->internalMut, NULL);
  sem_init(&cond->internalSem, 0, 0);
  return 0;
}

int
custom_cond_wait(custom_cond *cond, pthread_mutex_t *mutex)
{
  pthread_mutex_lock(&cond->internalMut);
  cond->numWaiters++;
  pthread_mutex_unlock(mutex);
  pthread_mutex_unlock(&cond->internalMut);
  sem_wait(&cond->internalSem);
  pthread_mutex_lock(mutex);
  return 0;
}

int
custom_cond_signal(custom_cond *cond)
{
  pthread_mutex_lock(&cond->internalMut);
  if (cond->numWaiters > 0) {
    cond->numWaiters--;
    sem_post(&cond->internalSem);
  }
  pthread_mutex_lock(&cond->internalMut);
  return 0;
}

int
custom_cond_broadcast(custom_cond *cond)
{
  pthread_mutex_lock(&cond->internalMut);

  if (cond->numWaiters > 0) {
    cond->numWaiters--;
    sem_post(&cond->internalSem);
  }

  pthread_mutex_lock(&cond->internalMut);
  return 0;
}