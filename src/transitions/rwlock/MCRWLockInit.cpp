#include "transitions/rwlock/MCRWLockInit.h"
#include "mcmini_private.h"

MCTransition *
MCReadRWLockInit(const MCSharedTransition *shmTransition,
                 void *shmData, MCStack *state)
{
  auto rwlockInShm = static_cast<MCRWLockShadow *>(shmData);
  auto systemId    = (MCSystemID)rwlockInShm->systemIdentity;
  auto rwLock =
    state->getVisibleObjectWithSystemIdentity<MCRWLock>(systemId);

  if (rwLock == nullptr) {
    auto newRWLock = std::make_shared<MCRWLock>(
      *rwlockInShm, MCRWLock::Type::no_preference);
    state->registerVisibleObjectWithSystemIdentity(systemId,
                                                   newRWLock);
    rwLock = newRWLock;
  }

  rwLock -> shadow.state = MCRWLockShadow::State::unlocked;

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCRWLockInit(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWLockInit::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWLock, MCVisibleObject>(
      this->rwlock->copy());
  return std::make_shared<MCRWLockInit>(threadCpy, rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWLockInit::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWLock> liveRWLock =
    state->getObjectWithId<MCRWLock>(rwlock->getObjectId());
  return std::make_shared<MCRWLockInit>(threadInState, liveRWLock);
}

void
MCRWLockInit::applyToState(MCStack *state)
{
  this->rwlock->init();
}

bool
MCRWLockInit::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCRWLockInit::dependentWith(const MCTransition *other) const
{
  const MCRWLockTransition *maybeRWLockOperation =
    dynamic_cast<const MCRWLockTransition *>(other);
  if (maybeRWLockOperation) {
    return *maybeRWLockOperation->rwlock == *this->rwlock;
  }
  return false;
}

void
MCRWLockInit::print() const
{
  mcprintf("thread %lu: pthread_rwlock_init(rwl:%u, _)\n", this->thread->tid,
           countVisibleObjectsOfType(this->rwlock->getObjectId()));
}
