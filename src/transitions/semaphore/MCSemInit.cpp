#include "mcmini/transitions/semaphore/MCSemInit.h"
#include "mcmini/mcmini_private.h"

MCTransition *
MCReadSemInit(const MCSharedTransition *shmTransition, void *shmData,
              MCStack *state)
{
  auto semInShm = static_cast<MCSemaphoreShadow *>(shmData);
  auto systemId = (MCSystemID)semInShm->sem;
  auto semThatExists =
    state->getVisibleObjectWithSystemIdentity<MCSemaphore>(systemId);

  if (semThatExists == nullptr) {
    auto newSem          = new MCSemaphore(*semInShm);
    auto newSemSharedPtr = std::shared_ptr<MCVisibleObject>(newSem);
    state->registerVisibleObjectWithSystemIdentity(systemId,
                                                   newSemSharedPtr);
    semThatExists =
      std::static_pointer_cast<MCSemaphore, MCVisibleObject>(
        newSemSharedPtr);
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCSemInit(threadThatRan, semThatExists);
}

std::shared_ptr<MCTransition>
MCSemInit::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto semCpy =
    std::static_pointer_cast<MCSemaphore, MCVisibleObject>(
      this->sem->copy());
  auto mutexInit = new MCSemInit(threadCpy, semCpy);
  return std::shared_ptr<MCTransition>(mutexInit);
}

std::shared_ptr<MCTransition>
MCSemInit::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCSemaphore> semInState =
    state->getObjectWithId<MCSemaphore>(sem->getObjectId());
  auto cpy = new MCSemInit(threadInState, semInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCSemInit::applyToState(MCStack *state)
{
  this->sem->init();
}

bool
MCSemInit::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCSemInit::dependentWith(const MCTransition *other) const
{
  return false;
}

void
MCSemInit::print() const
{
  printf("thread %lu: sem_init(sem:%u, 0, %u)\n", this->thread->tid,
         countVisibleObjectsOfType(this->sem->getObjectId()),
         this->sem->getCount());
}
