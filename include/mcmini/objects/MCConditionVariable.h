#ifndef INCLUDE_MCMINI_OBJECTS_MCCONDITIONVARIABLE_HPP
#define INCLUDE_MCMINI_OBJECTS_MCCONDITIONVARIABLE_HPP

#include "mcmini/misc/MCOptional.h"
#include "mcmini/misc/cond/MCConditionVariablePolicy.hpp"
#include "mcmini/objects/MCMutex.h"
#include "mcmini/objects/MCVisibleObject.h"
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

private:

  Shadow shadow;
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
  // Note it is undefined to access a single condition variable with
  // two different locks
  std::shared_ptr<MCMutex> mutex;

  ConditionVariable(Shadow shadow,
                    std::unique_ptr<ConditionVariablePolicy> policy,
                    std::shared_ptr<MCMutex> mutex = nullptr)
    : ConditionVariable(shadow, std::move(mutex), std::move(policy),
                        static_cast<objid_t>(0))
  {}

  ConditionVariable(const ConditionVariable &cond)
    : MCVisibleObject(cond.getObjectId()), shadow(cond.shadow),
      mutex(nullptr), policy(std::move(cond.policy->clone())),
      numRemainingSpuriousWakeups(cond.numRemainingSpuriousWakeups)
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
  void removeWaiter(tid_t tid);
  bool waiterCanExit(tid_t tid);
  void sendSignalMessage();
  void sendBroadcastMessage();
};

} // namespace mcmini

using MCConditionVariable       = mcmini::ConditionVariable;
using MCConditionVariableShadow = mcmini::ConditionVariable::Shadow;

#endif // INCLUDE_MCMINI_OBJECTS_MCCONDITIONVARIABLE_HPP
