#ifndef INCLUDE_MCMINI_OBJECTS_MCCONDITIONVARIABLE_HPP
#define INCLUDE_MCMINI_OBJECTS_MCCONDITIONVARIABLE_HPP

#include "mcmini/misc/MCOptional.h"
#include "mcmini/misc/cond/MCConditionVariableSignalPolicy.hpp"
#include "mcmini/misc/cond/MCConditionVariableWakeupPolicy.hpp"
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

  /**
   * @brief
   *
   */
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

  unsigned int numRemainingSpuriousWakeups = 1;

  /**
   * @brief The C-like shadow struct describing the basic state of
   * the condition variable
   */
  Shadow condShadow;

  std::unique_ptr<ConditionVariableSignalPolicy> signalPolicy;
  std::unique_ptr<ConditionVariableWakeupPolicy> wakeupPolicy;

  inline explicit ConditionVariable(
    Shadow condShadow, std::shared_ptr<MCMutex> mutex,
    std::unique_ptr<ConditionVariableSignalPolicy> &signalPolicy,
    std::unique_ptr<ConditionVariableWakeupPolicy> &wakeupPolicy,
    objid_t id)
    : MCVisibleObject(id), condShadow(condShadow), mutex(mutex),
      signalPolicy(std::move(signalPolicy)),
      wakeupPolicy(std::move(wakeupPolicy))
  {}

public:

  // The lock that is used to gain access to this condition variable
  // This value may be NULL before the condition variable has received
  // a pthread_cond_wait call
  //
  // Note it is undefined to access a single condition variable with
  // two different locks
  std::shared_ptr<MCMutex> mutex;

  inline explicit ConditionVariable(
    Shadow condShadow,
    std::unique_ptr<ConditionVariableSignalPolicy> &signalPolicy,
    std::unique_ptr<ConditionVariableWakeupPolicy> &wakeupPolicy)
    : ConditionVariable(condShadow, signalPolicy, wakeupPolicy,
                        nullptr)
  {}

  inline explicit ConditionVariable(
    Shadow condShadow,
    std::unique_ptr<ConditionVariableSignalPolicy> &signalPolicy,
    std::unique_ptr<ConditionVariableWakeupPolicy> &wakeupPolicy,
    std::shared_ptr<MCMutex> mutex)
    : MCVisibleObject(), condShadow(condShadow), mutex(mutex),
      signalPolicy(std::move(signalPolicy)),
      wakeupPolicy(std::move(wakeupPolicy))
  {}

  inline ConditionVariable(const ConditionVariable &cond)
    : MCVisibleObject(cond.getObjectId()),
      condShadow(cond.condShadow), mutex(nullptr),
      wakeupPolicy(cond.wakeupPolicy->clone()),
      signalPolicy(cond.signalPolicy->clone()),
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
