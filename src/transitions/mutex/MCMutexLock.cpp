#include "MCMINI_Private.h"
#include "MCMutexLock.h"
#include "transitions/threads/MCThreadCreate.h"
#include "MCMutexUnlock.h"
#include <memory>

MCTransition*
MCReadMutexLock(const MCSharedTransition *shmTransition, void *shmData, MCState *state)
{
    auto mutexInShm = static_cast<MCMutexShadow*>(shmData);
    auto mutexThatExists = state->getVisibleObjectWithSystemIdentity<MCMutex>((MCSystemID)mutexInShm->systemIdentity);

    MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(mutexThatExists != nullptr, "Attempting to lock an uninitialized mutex");
    if (mutexThatExists->isDestroyed()) {
        MC_REPORT_UNDEFINED_BEHAVIOR("Attempting to lock a mutex that has been destroyed");
    }

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new MCMutexLock(threadThatRan, mutexThatExists);
}

std::shared_ptr<MCTransition>
MCMutexLock::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<MCThread, MCVisibleObject>(this->thread->copy());
    auto mutexCpy =
            std::static_pointer_cast<MCMutex, MCVisibleObject>(this->mutex->copy());
    auto mutexLock = new MCMutexLock(threadCpy, mutexCpy);
    return std::shared_ptr<MCTransition>(mutexLock);
}

std::shared_ptr<MCTransition>
MCMutexLock::dynamicCopyInState(const MCState *state)
{
    std::shared_ptr<MCThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<MCMutex> mutexInState = state->getObjectWithId<MCMutex>(mutex->getObjectId());
    auto cpy = new MCMutexLock(threadInState, mutexInState);
    return std::shared_ptr<MCTransition>(cpy);
}

void
MCMutexLock::applyToState(MCState *state)
{
    this->mutex->lock(this->getThreadId());
}

bool
MCMutexLock::enabledInState(const MCState *state)
{
    return this->thread->enabled() && this->mutex->canAcquire(this->getThreadId());
}

bool
MCMutexLock::coenabledWith(std::shared_ptr<MCTransition> transition)
{
    {
        auto maybeMutexUnlock = std::dynamic_pointer_cast<MCMutexUnlock, MCTransition>(transition);
        if (maybeMutexUnlock) {
            return *maybeMutexUnlock->mutex != *this->mutex;
        }
    }
    return true;
}

bool
MCMutexLock::dependentWith(std::shared_ptr<MCTransition> transition)
{
    {
        auto maybeMutexLock = std::dynamic_pointer_cast<MCMutexLock, MCTransition>(transition);
        if (maybeMutexLock) {
            return *maybeMutexLock->mutex == *this->mutex;
        }
    }

    {
        auto maybeMutexUnlock = std::dynamic_pointer_cast<MCMutexUnlock, MCTransition>(transition);
        if (maybeMutexUnlock) {
            return *maybeMutexUnlock->mutex == *this->mutex;
        }
    }

    return false;
}

void
MCMutexLock::print()
{
    printf("thread %lu: pthread_mutex_lock(%lu)\n", this->thread->tid, this->mutex->getObjectId());
}