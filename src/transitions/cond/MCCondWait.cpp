#include "mcmini/transitions/cond/MCCondWait.h"
#include "mcmini/MCTransitionFactory.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/mutex/MCMutexLock.h"
#include "mcmini/transitions/mutex/MCMutexTransition.h"

MCTransition *
MCReadCondWait(const MCSharedTransition *shmTransition, void *shmData,
               MCState *state)
{
  const auto shmCond =
    static_cast<MCSharedMemoryConditionVariable *>(shmData);
  const auto condInShm     = shmCond->cond;
  const auto mutexInShm    = shmCond->mutex;
  const auto condSystemId  = (MCSystemID)condInShm;
  const auto mutexSystemId = (MCSystemID)mutexInShm;
  const auto condThatExists =
    state->getVisibleObjectWithSystemIdentity<MCConditionVariable>(
      condSystemId);
  const auto mutexThatExists =
    state->getVisibleObjectWithSystemIdentity<MCMutex>(mutexSystemId);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    condThatExists != nullptr, "Attempting to wait on a condition "
                               "variable that is uninitialized");
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    mutexThatExists != nullptr,
    "Attempting to wait on a condition variable with an "
    "uninitialized mutex");
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    !condThatExists->isDestroyed(),
    "Attempting to wait on a destroyed condition variable");

  const auto threadThatRanId = shmTransition->executor;
  const auto mutexAssociatedWithConditionVariable =
    condThatExists->mutex;

  if (mutexAssociatedWithConditionVariable != nullptr) {
    MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
      *mutexThatExists == *mutexAssociatedWithConditionVariable,
      "A mutex has already been associated with this condition "
      "variable. Attempting "
      "to use another mutex with the same condition variable is "
      "undefined");
  }

  auto threadThatRan = state->getThreadWithId(threadThatRanId);
  return new MCCondWait(threadThatRan, condThatExists);
}

std::shared_ptr<MCTransition>
MCCondWait::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto condCpy =
    std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
      this->conditionVariable->copy());
  auto cpy = new MCCondWait(threadCpy, condCpy);
  return std::shared_ptr<MCTransition>(cpy);
}

std::shared_ptr<MCTransition>
MCCondWait::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCConditionVariable> condInState =
    state->getObjectWithId<MCConditionVariable>(
      conditionVariable->getObjectId());
  auto cpy = new MCCondWait(threadInState, condInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCCondWait::applyToState(MCState *state)
{
  const tid_t threadId = this->getThreadId();
  this->conditionVariable->mutex->lock(threadId);
  this->conditionVariable->removeWaiter(threadId);
}

bool
MCCondWait::enabledInState(const MCState *) const
{
  const tid_t threadId = this->getThreadId();
  return this->conditionVariable->waiterCanExit(threadId) &&
         this->conditionVariable->mutex->canAcquire(threadId);
}

bool
MCCondWait::coenabledWith(const MCTransition *other) const
{
  auto maybeCondWaitOperation =
    dynamic_cast<const MCCondWait *>(other);
  if (maybeCondWaitOperation) {
    /* Only one cond_wait will be able to acquire the mutex */
    return *maybeCondWaitOperation->conditionVariable !=
           *this->conditionVariable;
  }

  const MCMutexTransition *maybeMutexOperation =
    dynamic_cast<const MCMutexTransition *>(other);
  if (maybeMutexOperation) {
    auto lockMutex = std::make_shared<MCMutexLock>(
      this->thread, this->conditionVariable->mutex);
    return MCTransition::coenabledTransitions(lockMutex.get(),
                                              maybeMutexOperation);
  }

  return true;
}

bool
MCCondWait::dependentWith(const MCTransition *other) const
{
  const MCCondTransition *maybeCondOperation =
    dynamic_cast<const MCCondTransition *>(other);
  if (maybeCondOperation) {
    return *maybeCondOperation->conditionVariable ==
           *this->conditionVariable;
  }

  const MCMutexTransition *maybeMutexOperation =
    dynamic_cast<const MCMutexTransition *>(other);
  if (maybeMutexOperation) {
    auto lockMutex = std::make_shared<MCMutexLock>(
      this->thread, this->conditionVariable->mutex);
    return MCTransition::coenabledTransitions(lockMutex.get(),
                                              maybeMutexOperation);
  }
  return false;
}

void
MCCondWait::print() const
{
  printf(
    "thread %lu: pthread_cond_wait(%lu, %lu) (asleep -> awake)\n",
    this->thread->tid, this->conditionVariable->getObjectId(),
    this->conditionVariable->mutex->getObjectId());
}
