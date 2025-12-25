#include "transitions/mutex/MCMutexUnlock.h"
#include "MCTransitionFactory.h"
#include "mcmini_private.h"

MCTransition *
MCReadMutexUnlock(const MCSharedTransition *shmTransition,
                  void *shmData, MCStack *state)
{
  auto mutexInShm = static_cast<MCMutexShadow *>(shmData);
  auto mutexThatExists =
    state->getVisibleObjectWithSystemIdentity<MCMutex>(
      (MCSystemID)mutexInShm->systemIdentity);
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    mutexThatExists != nullptr,
    "Attempting to unlock an uninitialized mutex");

  // 1. We've now computed mutexThatExists (i.e., the state of the
  //    mutex arguments to pthread_mutex_lock()).  Compute the nextTransition.
  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  MCTransition *nextTransition =
    new MCMutexUnlock(threadThatRan, mutexThatExists);

  // 2. Now test for undefined behaviors or invalid arguments.
  //    If fail, then call setNextTransition on nextTransition, in order to
  //    display correct results for programState->printNextTransitions().
  if (mutexThatExists == nullptr) {
    programState->setNextTransitionForThread(threadThatRanId, std::shared_ptr<MCTransition>(nextTransition));
    mc_report_undefined_behavior(
      "Attempting to unlock an uninitialized mutex");
  }
  if (mutexThatExists->isDestroyed()) {
    programState->setNextTransitionForThread(threadThatRanId, std::shared_ptr<MCTransition>(nextTransition));
    mc_report_undefined_behavior(
      "Attempting to unlock a mutex that has been destroyed");
  }
  if (mutexThatExists->isUnlocked()) {
    programState->setNextTransitionForThread(threadThatRanId, std::shared_ptr<MCTransition>(nextTransition));
    mc_report_undefined_behavior(
      "Attempting to unlock a mutex that is already unlocked");
  }
  if (mutexThatExists->ownerTid() != threadThatRanId) {
    programState->setNextTransitionForThread(threadThatRanId, std::shared_ptr<MCTransition>(nextTransition));
    mc_report_undefined_behavior(
      "Attempting to unlock a mutex that was locked by a different thread.");
  }

  // 3. Success (no undefined behavior or invalid args).  Return nextTransition.
  return nextTransition;
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
MCMutexUnlock::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCMutex> mutexInState =
    state->getObjectWithId<MCMutex>(mutex->getObjectId());
  auto cpy = new MCMutexUnlock(threadInState, mutexInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCMutexUnlock::applyToState(MCStack *state)
{
  this->mutex->unlock();
}

void
MCMutexUnlock::unapplyToState(MCStack *state)
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
MCMutexUnlock::isReversibleInState(const MCStack *state) const
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

MCTransitionUniqueRep
MCMutexUnlock::toUniqueRep() const
{
  MCTransitionUniqueRep uniqueRep;
  uniqueRep.typeId = MC_MUTEX_UNLOCK;
  uniqueRep.threadId = this->thread->tid;
  uniqueRep.param.val[0] = countVisibleObjectsOfType(this->mutex->getObjectId());
  uniqueRep.param.val[1] = UINT_MAX;
  return uniqueRep;
}

void
MCMutexUnlock::print() const
{
  mcprintf("thread %lu: pthread_mutex_unlock(mut:%u)\n", this->thread->tid,
           countVisibleObjectsOfType(this->mutex->getObjectId()));
}
