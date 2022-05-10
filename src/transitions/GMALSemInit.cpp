#include "GMALSemInit.h"

GMALTransition*
GMALReadSemInit(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    auto semInShm = static_cast<GMALSemaphoreShadow*>(shmData);
    auto systemId = (GMALSystemID)semInShm->sem;
    auto semThatExists = state->getVisibleObjectWithSystemIdentity<GMALSemaphore>(systemId);

    if (semThatExists == nullptr) {
        auto newSem = new GMALSemaphore(*semInShm);
        auto newSemSharedPtr = std::shared_ptr<GMALVisibleObject>(newSem);
        state->registerVisibleObjectWithSystemIdentity(systemId, newSemSharedPtr);
        semThatExists = std::static_pointer_cast<GMALSemaphore, GMALVisibleObject>(newSemSharedPtr);
    }

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALSemInit(threadThatRan, semThatExists);
}

std::shared_ptr<GMALTransition>
GMALSemInit::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto semCpy =
            std::static_pointer_cast<GMALSemaphore, GMALVisibleObject>(this->sem->copy());
    auto mutexInit = new GMALSemInit(threadCpy, semCpy);
    return std::shared_ptr<GMALTransition>(mutexInit);
}

std::shared_ptr<GMALTransition>
GMALSemInit::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALSemaphore> semInState = state->getObjectWithId<GMALSemaphore>(sem->getObjectId());
    auto cpy = new GMALSemInit(threadInState, semInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALSemInit::applyToState(GMALState *state)
{
    this->sem->init();
}

bool
GMALSemInit::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    return true;
}

bool
GMALSemInit::dependentWith(std::shared_ptr<GMALTransition> other)
{
    return false;
}

void
GMALSemInit::print()
{
    printf("thread %lu: sem_init(%lu, 0, %u)\n", this->thread->tid, this->sem->getObjectId(), this->sem->getCount());
}
