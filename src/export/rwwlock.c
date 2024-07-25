#include "export/rwwlock.h"

// This proves that McMini behaves as an emulator:
// A user can link this code into the target application.  We don't even need
// to define an implementation for when the target calls the functions here.
// McMini already models these functions as desired, and will continue
// to block or unblock threads according to a spec analogous to rwlock
// for POSIX.  Since McMini has the semantics correct, it will just work!

int
pthread_rwwlock_init(pthread_rwwlock_t *rwwlock)
{ return 0; }
int
pthread_rwwlock_rdlock(pthread_rwwlock_t *rwwlock)
{ return 0; }
int
pthread_rwwlock_wr1lock(pthread_rwwlock_t *rwwlock)
{ return 0; }
int
pthread_rwwlock_wr2lock(pthread_rwwlock_t *rwwlock)
{ return 0; }
int
pthread_rwwlock_unlock(pthread_rwwlock_t *rwwlock)
{ return 0; }
int
pthread_rwwlock_destroy(pthread_rwwlock_t *rwwlock)
{ return 0; }
