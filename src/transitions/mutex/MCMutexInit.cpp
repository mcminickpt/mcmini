#include "transitions/mutex/MCMutexInit.h"
#include "mcmini_private.h"

MCTransition *
MCReadMutexInit(const MCSharedTransition *shmTransition,
                void *shmData, MCStack *state)
{
  auto mutexInShm = static_cast<MCMutexShadow *>(shmData);
  auto systemId   = (MCSystemID)mutexInShm->systemIdentity;
  auto mutexThatExists =
    state->getVisibleObjectWithSystemIdentity<MCMutex>(systemId);

  if (mutexThatExists == nullptr) {
    auto newMutex = new MCMutex(*mutexInShm);
    auto newMutexSharedPtr =
      std::shared_ptr<MCVisibleObject>(newMutex);
    state->registerVisibleObjectWithSystemIdentity(systemId,
                                                   newMutexSharedPtr);
    mutexThatExists =
      std::static_pointer_cast<MCMutex, MCVisibleObject>(
        newMutexSharedPtr);
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCMutexInit(threadThatRan, mutexThatExists);
}

std::shared_ptr<MCTransition>
MCMutexInit::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto mutexCpy = std::static_pointer_cast<MCMutex, MCVisibleObject>(
    this->mutex->copy());
  auto mutexInit = new MCMutexInit(threadCpy, mutexCpy);
  return std::shared_ptr<MCTransition>(mutexInit);
}

std::shared_ptr<MCTransition>
MCMutexInit::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCMutex> mutexInState =
    state->getObjectWithId<MCMutex>(mutex->getObjectId());
  auto cpy = new MCMutexInit(threadInState, mutexInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCMutexInit::applyToState(MCStack *state)
{
  this->mutex->init();
}

void
MCMutexInit::unapplyToState(MCStack *state)
{
  this->mutex->deinit();
}

bool
MCMutexInit::isReversibleInState(const MCStack *state) const
{
  return true;
}

bool
MCMutexInit::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCMutexInit::dependentWith(const MCTransition *other) const
{
  const MCMutexTransition *maybeMutexOperation =
    dynamic_cast<const MCMutexTransition *>(other);
  if (maybeMutexOperation) {
    return *maybeMutexOperation->mutex == *this->mutex;
  }
  return false;
}

MCTransitionUniqueRep
MCMutexInit::toUniqueRep() const
{
  MCTransitionUniqueRep uniqueRep;
  uniqueRep.typeId = MC_MUTEX_INIT;
  uniqueRep.threadId = this->thread->tid;
  uniqueRep.param.val[0] = countVisibleObjectsOfType(this->mutex->getObjectId());
  uniqueRep.param.val[1] = UINT_MAX;
  return uniqueRep;
}

void
MCMutexInit::print() const
{
  mcprintf("thread %lu: pthread_mutex_init(mut:%u, _)\n",
           this->thread->tid,
           countVisibleObjectsOfType(this->mutex->getObjectId()));
}
