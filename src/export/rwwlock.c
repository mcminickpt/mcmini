#include "mcmini/export/rwwlock.h"

// This proves that McMini behaves as an emulator:
// we don't even need to define implementations
// for the true function calls here. As long as
// McMini has the semantics correct, it will
// just work!

int
pthread_rwwlock_init(pthread_rwwlock_t *rwwlock)
{}
int
pthread_rwwlock_rdlock(pthread_rwwlock_t *rwwlock)
{}
int
pthread_rwwlock_wr1lock(pthread_rwwlock_t *rwwlock)
{}
int
pthread_rwwlock_wr2lock(pthread_rwwlock_t *rwwlock)
{}
int
pthread_rwwlock_unlock(pthread_rwwlock_t *rwwlock)
{}
int
pthread_rwwlock_destroy(pthread_rwwlock_t *rwwlock)
{}