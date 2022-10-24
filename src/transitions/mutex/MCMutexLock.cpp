#include "mcmini/transitions/mutex/MCMutexLock.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/mutex/MCMutexUnlock.h"
#include "mcmini/transitions/threads/MCThreadCreate.h"
#include <memory>

MCTransition *
MCReadMutexLock(const MCSharedTransition *shmTransition,
                void *shmData, MCState *state)
{
  auto mutexInShm = static_cast<MCMutexShadow *>(shmData);
  auto mutexThatExists =
    state->getVisibleObjectWithSystemIdentity<MCMutex>(
      (MCSystemID)mutexInShm->systemIdentity);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    mutexThatExists != nullptr,
    "Attempting to lock an uninitialized mutex");
  if (mutexThatExists->isDestroyed()) {
    MC_REPORT_UNDEFINED_BEHAVIOR(
      "Attempting to lock a mutex that has been destroyed");
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCMutexLock(threadThatRan, mutexThatExists);
}

std::shared_ptr<MCTransition>
MCMutexLock::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto mutexCpy = std::static_pointer_cast<MCMutex, MCVisibleObject>(
    this->mutex->copy());
  return std::make_shared<MCMutexLock>(threadCpy, mutexCpy);
}

std::shared_ptr<MCTransition>
MCMutexLock::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCMutex> mutexInState =
    state->getObjectWithId<MCMutex>(mutex->getObjectId());
  return std::make_shared<MCMutexLock>(threadInState, mutexInState);
}

void
MCMutexLock::applyToState(MCState *state)
{
  this->mutex->lock(this->getThreadId());
}

void
MCMutexLock::unapplyToState(MCState *state)
{
  this->mutex->unlock();
}

bool
MCMutexLock::isReversibleInState(const MCState *state) const
{
  return false;
}

bool
MCMutexLock::enabledInState(const MCState *state) const
{
  return this->mutex->canAcquire(this->getThreadId());
}

bool
MCMutexLock::coenabledWith(const MCTransition *transition) const
{
  {
    const MCMutexUnlock *maybeMutexUnlock =
      dynamic_cast<const MCMutexUnlock *>(transition);
    if (maybeMutexUnlock) {
      return *maybeMutexUnlock->mutex != *this->mutex;
    }
  }
  return true;
}

bool
MCMutexLock::dependentWith(const MCTransition *transition) const
{
  {
    const MCMutexLock *maybeMutexLock =
      dynamic_cast<const MCMutexLock *>(transition);
    if (maybeMutexLock) {
      return *maybeMutexLock->mutex == *this->mutex;
    }
  }

  {
    const MCMutexUnlock *maybeMutexUnlock =
      dynamic_cast<const MCMutexUnlock *>(transition);
    if (maybeMutexUnlock) {
      return *maybeMutexUnlock->mutex == *this->mutex;
    }
  }

  return false;
}

void
MCMutexLock::print() const
{
  printf("thread %lu: pthread_mutex_lock(%lu)\n", this->thread->tid,
         this->mutex->getObjectId());
}