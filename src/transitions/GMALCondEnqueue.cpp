#include "GMALCondEnqueue.h"
#include "GMAL.h"
#include "GMALMutexTransition.h"
#include "GMALMutexUnlock.h"
#include "GMALTransitionFactory.h"

GMALTransition*
GMALReadCondEnqueue(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
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
    return new GMALCondEnqueue(threadThatRan, condThatExists, mutexThatExists);
}

std::shared_ptr<GMALTransition>
GMALCondEnqueue::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto condCpy =
            std::static_pointer_cast<GMALConditionVariable, GMALVisibleObject>(this->conditionVariable->copy());
    auto mutCpy =
            std::static_pointer_cast<GMALMutex, GMALVisibleObject>(this->mutex->copy());
    auto cpy = new GMALCondEnqueue(threadCpy, condCpy, mutCpy);
    return std::shared_ptr<GMALTransition>(cpy);
}

std::shared_ptr<GMALTransition>
GMALCondEnqueue::dynamicCopyInState(const GMALState *state)
{
    auto threadInState = state->getThreadWithId(thread->tid);
    auto condInState = state->getObjectWithId<GMALConditionVariable>(conditionVariable->getObjectId());
    auto mutCpy = state->getObjectWithId<GMALMutex>(this->mutex->getObjectId());
    auto cpy = new GMALCondEnqueue(threadInState, condInState, mutCpy);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALCondEnqueue::applyToState(GMALState *state)
{
    /* Insert this thread into the waiting queue */
    this->conditionVariable->enterSleepingQueue(this->getThreadId());
    this->conditionVariable->mutex = this->mutex;
    this->mutex->unlock();
}

bool
GMALCondEnqueue::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeCondWaitOperation = std::dynamic_pointer_cast<GMALCondEnqueue, GMALTransition>(other);
    if (maybeCondWaitOperation) {
        /* Only one cond_wait will be able to acquire the mutex */
        return *maybeCondWaitOperation->conditionVariable != *this->conditionVariable;
    }

    auto maybeMutexOperation = std::dynamic_pointer_cast<GMALMutexTransition, GMALTransition>(other);
    if (maybeMutexOperation) {
        auto unlockMutex = std::make_shared<GMALMutexUnlock>(this->thread, this->conditionVariable->mutex);
        return GMALTransitionFactory::transitionsCoenabledCommon(unlockMutex, maybeMutexOperation);
    }

    return true;
}

bool
GMALCondEnqueue::dependentWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeCondOperation = std::dynamic_pointer_cast<GMALCondTransition, GMALTransition>(other);
    if (maybeCondOperation) {
        return *maybeCondOperation->conditionVariable == *this->conditionVariable;
    }

    auto maybeMutexOperation = std::dynamic_pointer_cast<GMALMutexTransition, GMALTransition>(other);
    if (maybeMutexOperation) {
        auto unlockMutex = std::make_shared<GMALMutexUnlock>(this->thread, this->conditionVariable->mutex);
        return GMALTransitionFactory::transitionsCoenabledCommon(unlockMutex, maybeMutexOperation);
    }
    return false;
}

void
GMALCondEnqueue::print()
{
    printf("thread %lu: pthread_cond_wait(%lu, %lu) (enter)\n", this->thread->tid, this->conditionVariable->getObjectId(), this->mutex->getObjectId());
}
