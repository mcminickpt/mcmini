#include "mcmini/transitions/mutex/MCMutexUnlock.h"
#include "mcmini/MCTransitionFactory.h"
#include "mcmini/mcmini_private.h"

MCTransition *
MCReadMutexUnlock(const MCSharedTransition *shmTransition,
                  void *shmData, MCState *state)
{
  auto mutexInShm = static_cast<MCMutexShadow *>(shmData);
  auto mutexThatExists =
    state->getVisibleObjectWithSystemIdentity<MCMutex>(
      (MCSystemID)mutexInShm->systemIdentity);
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    mutexThatExists != nullptr,
    "Attempting to unlock an uninitialized mutex");

  if (mutexThatExists->isUnlocked()) {
    MC_REPORT_UNDEFINED_BEHAVIOR(
      "Attempting to unlock a mutex that is already unlocked");
  } else if (mutexThatExists->isDestroyed()) {
    MC_REPORT_UNDEFINED_BEHAVIOR(
      "Attempting to unlock a mutex that has been destroyed");
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCMutexUnlock(threadThatRan, mutexThatExists);
}

std::shared_ptr<MCTransition>
MCMutexUnlock::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto mutexCpy = std::static_pointer_cast<MCMutex, MCVisibleObject>(
    this->mutex->copy());
  auto mutexUnlock = new MCMutexUnlock(threadCpy, mutexCpy);
  return std::shared_ptr<MCTransition>(mutexUnlock);
}

std::shared_ptr<MCTransition>
MCMutexUnlock::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCMutex> mutexInState =
    state->getObjectWithId<MCMutex>(mutex->getObjectId());
  auto cpy = new MCMutexUnlock(threadInState, mutexInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCMutexUnlock::applyToState(MCState *state)
{
  this->mutex->unlock();
}

void
MCMutexUnlock::unapplyToState(MCState *state)
{
  // Assumes that we were holding onto the lock
  // before executing the unlock operation!
  //
  // If we execute an unlock from a different
  // thread, this is undefined behavior that
  // McMini should hopefully report before
  // we'd ever reach a bad state like that
  this->mutex->lock(this->getThreadId());
}

bool
MCMutexUnlock::isReversibleInState(const MCState *state) const
{
  return true;
}

bool
MCMutexUnlock::coenabledWith(const MCTransition *transition) const
{
  return true;
}

bool
MCMutexUnlock::dependentWith(const MCTransition *transition) const
{
  return false;
}

void
MCMutexUnlock::print() const
{
  printf("thread %lu: pthread_mutex_unlock(%lu)\n", this->thread->tid,
         this->mutex->getObjectId());
}
