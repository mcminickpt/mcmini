#include "mcmini/transitions/rwwlock/MCRWWLockInit.h"
#include "mcmini/mcmini_private.h"

MCTransition *
MCReadRWWLockInit(const MCSharedTransition *shmTransition,
                  void *shmData, MCStack *state)
{
  auto rwlockInShm = static_cast<MCRWWLockShadow *>(shmData);
  auto systemId    = (MCSystemID)rwlockInShm->systemIdentity;
  auto rwLock =
    state->getVisibleObjectWithSystemIdentity<MCRWWLock>(systemId);

  if (rwLock == nullptr) {
    auto newRWLock = std::make_shared<MCRWWLock>(
      *rwlockInShm, MCRWWLock::Type::no_preference);
    state->registerVisibleObjectWithSystemIdentity(systemId,
                                                   newRWLock);
    rwLock = newRWLock;
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCRWWLockInit(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWWLockInit::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWWLock, MCVisibleObject>(
      this->rwwlock->copy());
  return std::make_shared<MCRWWLockInit>(threadCpy, rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWWLockInit::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWWLock> liveRWLock =
    state->getObjectWithId<MCRWWLock>(rwwlock->getObjectId());
  return std::make_shared<MCRWWLockInit>(threadInState, liveRWLock);
}

void
MCRWWLockInit::applyToState(MCStack *state)
{
  this->rwwlock->init();
}

bool
MCRWWLockInit::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCRWWLockInit::dependentWith(const MCTransition *other) const
{
  const MCRWWLockTransition *maybeRWLockOperation =
    dynamic_cast<const MCRWWLockTransition *>(other);
  if (maybeRWLockOperation) {
    return *maybeRWLockOperation->rwwlock == *this->rwwlock;
  }
  return false;
}

void
MCRWWLockInit::print() const
{
  printf("thread %lu: pthread_rwwlock_init(rwwl:%u, _)\n", this->thread->tid,
         countVisibleObjectsOfType(this->rwwlock->getObjectId()));
}
