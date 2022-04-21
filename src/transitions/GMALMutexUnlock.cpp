#include "GMALMutexUnlock.h"
#include "GMALTransitionFactory.h"

GMALTransition*
GMALReadMutexUnlock(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    auto mutexInShm = static_cast<GMALMutexShadow*>(shmData);
    auto mutexThatExists = state->getVisibleObjectWithSystemIdentity<GMALMutex>((GMALSystemID)mutexInShm->systemIdentity);

    // TODO: Figure out how to deal with undefined behavior
    GMAL_ASSERT(mutexThatExists != nullptr);

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALMutexUnlock(threadThatRan, mutexThatExists);
}

std::shared_ptr<GMALTransition>
GMALMutexUnlock::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto mutexCpy =
            std::static_pointer_cast<GMALMutex, GMALVisibleObject>(this->mutex->copy());
    auto mutexUnlock = new GMALMutexUnlock(threadCpy, mutexCpy);
    return std::shared_ptr<GMALTransition>(mutexUnlock);
}

std::shared_ptr<GMALTransition>
GMALMutexUnlock::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALMutex> mutexInState = state->getObjectWithId<GMALMutex>(mutex->getObjectId());
    auto cpy = new GMALMutexUnlock(threadInState, mutexInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALMutexUnlock::applyToState(GMALState *state)
{
    this->mutex->unlock();
}

void
GMALMutexUnlock::unapplyToState(GMALState *state)
{
    this->mutex->lock();
}

bool
GMALMutexUnlock::enabledInState(const GMALState *state)
{
    return this->thread->enabled();
}

bool
GMALMutexUnlock::coenabledWith(std::shared_ptr<GMALTransition> transition)
{
    return true;
}

bool
GMALMutexUnlock::dependentWith(std::shared_ptr<GMALTransition> transition)
{
    return false;
}
