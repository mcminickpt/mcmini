#include "GMAL.h"
#include "GMALMutexLock.h"
#include "GMALThreadCreate.h"
#include "GMALThreadJoin.h"
#include "GMALMutexUnlock.h"
#include <memory>

GMALTransition*
GMALReadMutexLock(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    auto mutexInShm = static_cast<GMALMutexShadow*>(shmData);
    auto mutexThatExists = state->getVisibleObjectWithSystemIdentity<GMALMutex>((GMALSystemID)mutexInShm->systemIdentity);

    GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(mutexThatExists != nullptr, "Attempting to lock an uninitialized mutex");
    if (mutexThatExists->isDestroyed()) {
        GMAL_REPORT_UNDEFINED_BEHAVIOR("Attempting to lock a mutex that has been destroyed");
    }

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALMutexLock(threadThatRan, mutexThatExists);
}

std::shared_ptr<GMALTransition>
GMALMutexLock::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto mutexCpy =
            std::static_pointer_cast<GMALMutex, GMALVisibleObject>(this->mutex->copy());
    auto mutexLock = new GMALMutexLock(threadCpy, mutexCpy);
    return std::shared_ptr<GMALTransition>(mutexLock);
}

std::shared_ptr<GMALTransition>
GMALMutexLock::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALMutex> mutexInState = state->getObjectWithId<GMALMutex>(mutex->getObjectId());
    auto cpy = new GMALMutexLock(threadInState, mutexInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALMutexLock::applyToState(GMALState *state)
{
    this->mutex->lock(this->getThreadId());
}

bool
GMALMutexLock::enabledInState(const GMALState *state)
{
    return this->thread->enabled() && this->mutex->canAcquire(this->getThreadId());
}

bool
GMALMutexLock::coenabledWith(std::shared_ptr<GMALTransition> transition)
{
    {
        auto maybeMutexUnlock = std::dynamic_pointer_cast<GMALMutexUnlock, GMALTransition>(transition);
        if (maybeMutexUnlock) {
            return *maybeMutexUnlock->mutex != *this->mutex;
        }
    }
    return true;
}

bool
GMALMutexLock::dependentWith(std::shared_ptr<GMALTransition> transition)
{
    {
        auto maybeMutexLock = std::dynamic_pointer_cast<GMALMutexLock, GMALTransition>(transition);
        if (maybeMutexLock) {
            return *maybeMutexLock->mutex == *this->mutex;
        }
    }

    {
        auto maybeMutexUnlock = std::dynamic_pointer_cast<GMALMutexUnlock, GMALTransition>(transition);
        if (maybeMutexUnlock) {
            return *maybeMutexUnlock->mutex == *this->mutex;
        }
    }

    return false;
}

void
GMALMutexLock::print()
{
    printf("thread %lu: pthread_mutex_lock(%lu)\n", this->thread->tid, this->mutex->getObjectId());
}