#include "GMAL.h"
#include "GMALMutexUnlock.h"
#include "GMALTransitionFactory.h"

GMALTransition*
GMALReadMutexUnlock(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    auto mutexInShm = static_cast<GMALMutexShadow*>(shmData);
    auto mutexThatExists = state->getVisibleObjectWithSystemIdentity<GMALMutex>((GMALSystemID)mutexInShm->systemIdentity);
    GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(mutexThatExists != nullptr, "Attempting to unlock an uninitialized mutex");

    if (mutexThatExists->isUnlocked()) {
        GMAL_REPORT_UNDEFINED_BEHAVIOR("Attempting to unlock a mutex that is already unlocked");
    } else if (mutexThatExists->isDestroyed()) {
        GMAL_REPORT_UNDEFINED_BEHAVIOR("Attempting to unlock a mutex that has been destroyed");
    }

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

void
GMALMutexUnlock::print()
{
    printf("thread %lu: pthread_mutex_unlock(%lu)\n", this->thread->tid, this->mutex->getObjectId());
}
