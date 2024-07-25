#ifndef INCLUDE_MCMINI_OBJECTS_MCCONDITIONVARIABLE_HPP
#define INCLUDE_MCMINI_OBJECTS_MCCONDITIONVARIABLE_HPP

#include "misc/MCOptional.h"
#include "misc/cond/MCConditionVariablePolicy.hpp"
#include "objects/MCMutex.h"
#include "objects/MCVisibleObject.h"
#include <deque>
#include <vector>

struct MCSharedMemoryConditionVariable {
  pthread_cond_t *cond;
  pthread_mutex_t *mutex;

  MCSharedMemoryConditionVariable(pthread_cond_t *cond,
                                  pthread_mutex_t *mutex)
    : cond(cond), mutex(mutex)
  {}
};

namespace mcmini {

struct ConditionVariable : public MCVisibleObject {
public:

  struct Shadow {
    pthread_cond_t *cond;

    enum State {
      undefined,
      initialized,
      destroyed
    } state = undefined;

    explicit Shadow(pthread_cond_t *cond) : cond(cond) {}
  };

  // `FIXME:  This used to be private.  But MCReadCondSignal, etc.,
  //          needs to set shadow.state to 'initialized'.
  Shadow shadow;

private:

  unsigned int numRemainingSpuriousWakeups = 0;
  std::unique_ptr<ConditionVariablePolicy> policy;

  inline explicit ConditionVariable(
    Shadow shadow, std::shared_ptr<MCMutex> mutex,
    std::unique_ptr<ConditionVariablePolicy> policy, objid_t id)
    : MCVisibleObject(id), shadow(shadow), policy(std::move(policy)),
      mutex(std::move(mutex))
  {}

public:

  // The lock that is used to gain access to this condition variable
  // This value may be NULL before the condition variable has received
  // a pthread_cond_wait call
  //
  // NOTE:  See MCCondWait.cpp:MCCondWait::applyToState() for a
  //   comment on a single condition variable being successively
  //   bound to two different mutexes.
  std::shared_ptr<MCMutex> mutex;

  ConditionVariable(Shadow shadow,
                    std::unique_ptr<ConditionVariablePolicy> policy,
                    std::shared_ptr<MCMutex> mutex = nullptr)
    : ConditionVariable(shadow, std::move(mutex), std::move(policy),
                        static_cast<objid_t>(0))
  {}

  ConditionVariable(const ConditionVariable &cond)
    : MCVisibleObject(cond.getObjectId()), shadow(cond.shadow),
      numRemainingSpuriousWakeups(cond.numRemainingSpuriousWakeups),
      policy(std::move(cond.policy->clone())),
      mutex(nullptr)
  {
    if (cond.mutex != nullptr) {
      mutex = std::static_pointer_cast<MCMutex, MCVisibleObject>(
        cond.mutex->copy());
    }
  }

  std::shared_ptr<MCVisibleObject> copy() override;
  MCSystemID getSystemId() override;

  bool operator==(const ConditionVariable &) const;
  bool operator!=(const ConditionVariable &) const;

  bool isInitialized() const;
  bool isDestroyed() const;

  void initialize();
  void destroy();

  void addWaiter(tid_t tid);
  bool hasWaiters();
  void removeWaiter(tid_t tid);
  bool waiterCanExit(tid_t tid);
  void sendSignalMessage();
  void sendBroadcastMessage();
};

} // namespace mcmini

using MCConditionVariable       = mcmini::ConditionVariable;
using MCConditionVariableShadow = mcmini::ConditionVariable::Shadow;

#endif // INCLUDE_MCMINI_OBJECTS_MCCONDITIONVARIABLE_HPP
