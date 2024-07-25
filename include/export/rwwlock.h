#ifndef INCLUDE_MCMINI_RWWLOCK_HPP
#define INCLUDE_MCMINI_RWWLOCK_HPP

typedef struct {
  /* Contents don't matter here */
} pthread_rwwlock_t;

/* Implementations don't matter: McMini's emulation will be enough */
int pthread_rwwlock_init(pthread_rwwlock_t *);
int pthread_rwwlock_rdlock(pthread_rwwlock_t *);
int pthread_rwwlock_wr1lock(pthread_rwwlock_t *);
int pthread_rwwlock_wr2lock(pthread_rwwlock_t *);
int pthread_rwwlock_unlock(pthread_rwwlock_t *);
int pthread_rwwlock_destroy(pthread_rwwlock_t *);

#endif // INCLUDE_MCMINI_RWWLOCK_HPP
