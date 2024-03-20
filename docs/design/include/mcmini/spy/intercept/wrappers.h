// wrappers.h

#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <pthread.h>
#include "mcmini/shared_transition.h"


// struct MCTransition {
//   // Include necessary information about the transition
//   // (This is a placeholder; you may extend it as needed)
//   std::shared_ptr<void> data;

//   MCTransition(/* Add relevant parameters */) {
//     // Initialize the transition data based on the parameters
//     // (This is a placeholder; you may extend it as needed)
//   }

//   // Implement additional functions if needed
// };

MC_EXTERN int mc_pthread_mutex_init(pthread_mutex_t *,
                                    const pthread_mutexattr_t *);


struct MCMutexShadow {
  pthread_mutex_t *systemIdentity;
  enum State { undefined, unlocked, locked, destroyed } state;
  explicit MCMutexShadow(pthread_mutex_t *systemIdentity)
    : systemIdentity(systemIdentity), state(undefined)
  {}
};

struct MCMutex : public MCVisibleObject {
private:

  MCMutexShadow mutexShadow;

public:

  inline explicit MCMutex(MCMutexShadow shadow)
    : MCVisibleObject(), mutexShadow(shadow)
  {}
  inline MCMutex(const MCMutex &mutex)
    : MCVisibleObject(mutex.getObjectId()),
      mutexShadow(mutex.mutexShadow)
  {}

  std::shared_ptr<MCVisibleObject> copy() override;
  MCSystemID getSystemId() override;

  bool operator==(const MCMutex &) const;
  bool operator!=(const MCMutex &) const;

  bool canAcquire(tid_t) const;
  bool isLocked() const;
  bool isUnlocked() const;
  bool isDestroyed() const;

  void lock(tid_t);
  void unlock();
  void init();
  void deinit();
};

#endif // WRAPPERS_H
