#include "mcmini/transitions/cond/MCCondEnqueue.h"
#include "mcmini/MCTransitionFactory.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/mutex/MCMutexTransition.h"
#include "mcmini/transitions/mutex/MCMutexUnlock.h"

MCTransition *
MCReadCondEnqueue(const MCSharedTransition *shmTransition,
                  void *shmData, MCState *state)
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

  // NOTE: It's possible (and likely) for a condition variable
  // to NOT already be associated with a mutex at this point.
  // E.g., the first call to pthread_cond_wait() will have a
  // condition variable that isn't associated with a mutex.
  if (mutexAssociatedWithConditionVariable) {
    MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
      *mutexAssociatedWithConditionVariable == *mutexThatExists,
      "A mutex has already been associated with this condition "
      "variable. Attempting to wait on a condition variable using "
      "more "
      "than one mutex is undefined");
  }

  // NOTE: We have to associate the mutex with the condition
  // variable when the transition is encountered; otherwise,
  // we wouldn't be able to determine if, e.g., a pthread_cond_wait()
  // enqueue call were dependent with a pthread_mutex_lock(). Note
  // that we ALSO must assign the mutex when the operation is APPLIED
  // to the condition variable
  condThatExists->mutex = mutexThatExists;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCCondEnqueue(threadThatRan, condThatExists,
                           mutexThatExists);
}

std::shared_ptr<MCTransition>
MCCondEnqueue::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto condCpy =
    std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
      this->conditionVariable->copy());
  auto mutCpy = std::static_pointer_cast<MCMutex, MCVisibleObject>(
    this->mutex->copy());
  auto cpy = new MCCondEnqueue(threadCpy, condCpy, mutCpy);
  return std::shared_ptr<MCTransition>(cpy);
}

std::shared_ptr<MCTransition>
MCCondEnqueue::dynamicCopyInState(const MCState *state) const
{
  auto threadInState = state->getThreadWithId(thread->tid);
  auto condInState   = state->getObjectWithId<MCConditionVariable>(
    conditionVariable->getObjectId());
  auto mutCpy =
    state->getObjectWithId<MCMutex>(this->mutex->getObjectId());
  auto cpy = new MCCondEnqueue(threadInState, condInState, mutCpy);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCCondEnqueue::applyToState(MCState *state)
{
  /* Insert this thread into the waiting queue */
  this->conditionVariable->addWaiter(this->getThreadId());
  this->conditionVariable->mutex = this->mutex;
  this->mutex->unlock();
}

bool
MCCondEnqueue::coenabledWith(const MCTransition *other) const
{
  const MCCondEnqueue *maybeCondEnqueueOperation =
    dynamic_cast<const MCCondEnqueue *>(other);
  if (maybeCondEnqueueOperation) {
    /* Only one cond_wait will be able to acquire the mutex */
    return *maybeCondEnqueueOperation->conditionVariable !=
           *this->conditionVariable;
  }

  const MCMutexTransition *maybeMutexOperation =
    dynamic_cast<const MCMutexTransition *>(other);
  if (maybeMutexOperation) {
    auto unlockMutex = std::make_shared<MCMutexUnlock>(
      this->thread, this->conditionVariable->mutex);
    return MCTransition::coenabledTransitions(unlockMutex.get(),
                                              maybeMutexOperation);
  }

  return true;
}

bool
MCCondEnqueue::dependentWith(const MCTransition *other) const
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
    auto unlockMutex = std::make_shared<MCMutexUnlock>(
      this->thread, this->conditionVariable->mutex);
    return MCTransition::coenabledTransitions(unlockMutex.get(),
                                              maybeMutexOperation);
  }
  return false;
}

void
MCCondEnqueue::print() const
{
  printf(
    "thread %lu: pthread_cond_wait(%lu, %lu) (awake -> asleep)\n",
    this->thread->tid, this->conditionVariable->getObjectId(),
    this->mutex->getObjectId());
}
