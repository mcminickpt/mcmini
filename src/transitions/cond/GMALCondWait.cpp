#include "GMALCondWait.h"
#include "GMAL.h"
#include "transitions/mutex/GMALMutexTransition.h"
#include "transitions/mutex/GMALMutexLock.h"
#include "GMALTransitionFactory.h"

GMALTransition*
GMALReadCondWait(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    const auto shmCond = static_cast<GMALSharedMemoryConditionVariable*>(shmData);
    const auto condInShm = shmCond->cond;
    const auto mutexInShm = shmCond->mutex;
    const auto condSystemId = (GMALSystemID)condInShm;
    const auto mutexSystemId = (GMALSystemID)mutexInShm;
    const auto condThatExists = state->getVisibleObjectWithSystemIdentity<GMALConditionVariable>(condSystemId);
    const auto mutexThatExists = state->getVisibleObjectWithSystemIdentity<GMALMutex>(mutexSystemId);

    GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(condThatExists != nullptr, "Attempting to wait on a condition variable that is uninitialized");
    GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(mutexThatExists != nullptr, "Attempting to wait on a condition variable with an uninitialized mutex");
    GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(!condThatExists->isDestroyed(), "Attempting to wait on a destroyed condition variable");

    const auto threadThatRanId = shmTransition->executor;
    const auto mutexAssociatedWithConditionVariable = condThatExists->mutex;

    if (mutexAssociatedWithConditionVariable != nullptr) {
        GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(*mutexThatExists == *mutexAssociatedWithConditionVariable,
                                               "A mutex has already been associated with this condition variable. Attempting "
                                               "to use another mutex with the same condition variable is undefined");
    }

    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALCondWait(threadThatRan, condThatExists);
}

std::shared_ptr<GMALTransition>
GMALCondWait::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto condCpy =
            std::static_pointer_cast<GMALConditionVariable, GMALVisibleObject>(this->conditionVariable->copy());
    auto cpy = new GMALCondWait(threadCpy, condCpy);
    return std::shared_ptr<GMALTransition>(cpy);
}

std::shared_ptr<GMALTransition>
GMALCondWait::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALConditionVariable> condInState = state->getObjectWithId<GMALConditionVariable>(conditionVariable->getObjectId());
    auto cpy = new GMALCondWait(threadInState, condInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALCondWait::applyToState(GMALState *state)
{
    const auto threadId = this->getThreadId();
    this->conditionVariable->mutex->lock(threadId);
    this->conditionVariable->removeThread(threadId); /* When we actually apply the wait, we are moving out of it */
}


bool
GMALCondWait::enabledInState(const GMALState *) {
    const auto threadId = this->getThreadId();
    return this->thread->enabled() &&
            this->conditionVariable->threadCanExit(threadId) &&
            this->conditionVariable->mutex->canAcquire(threadId);
}

bool
GMALCondWait::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeCondWaitOperation = std::dynamic_pointer_cast<GMALCondWait, GMALTransition>(other);
    if (maybeCondWaitOperation) {
        /* Only one cond_wait will be able to acquire the mutex */
        return *maybeCondWaitOperation->conditionVariable != *this->conditionVariable;
    }

    auto maybeMutexOperation = std::dynamic_pointer_cast<GMALMutexTransition, GMALTransition>(other);
    if (maybeMutexOperation) {
        auto lockMutex = std::make_shared<GMALMutexLock>(this->thread, this->conditionVariable->mutex);
        return GMALTransitionFactory::transitionsCoenabledCommon(lockMutex, maybeMutexOperation);
    }

    return true;
}

bool
GMALCondWait::dependentWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeCondOperation = std::dynamic_pointer_cast<GMALCondTransition, GMALTransition>(other);
    if (maybeCondOperation) {
        return *maybeCondOperation->conditionVariable == *this->conditionVariable;
    }

    auto maybeMutexOperation = std::dynamic_pointer_cast<GMALMutexTransition, GMALTransition>(other);
    if (maybeMutexOperation) {
        auto lockMutex = std::make_shared<GMALMutexLock>(this->thread, this->conditionVariable->mutex);
        return GMALTransitionFactory::transitionsDependentCommon(lockMutex, maybeMutexOperation);
    }
    return false;
}

void
GMALCondWait::print()
{
    printf("thread %lu: pthread_cond_wait(%lu, %lu) (asleep)\n", this->thread->tid, this->conditionVariable->getObjectId(), this->conditionVariable->mutex->getObjectId());
}
