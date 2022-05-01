#include "GMALCondInit.h"

GMALTransition*
GMALReadCondInit(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    auto condInShm = static_cast<GMALConditionVariableShadow*>(shmData);
    auto systemId = (GMALSystemID)condInShm->cond;
    auto condThatExists = state->getVisibleObjectWithSystemIdentity<GMALConditionVariable>(systemId);

    if (condThatExists == nullptr) {
        auto newCond = new GMALConditionVariable(*condInShm);
        auto newMutexSharedPtr = std::shared_ptr<GMALVisibleObject>(newCond);
        state->registerVisibleObjectWithSystemIdentity(systemId, newMutexSharedPtr);
        condThatExists = std::static_pointer_cast<GMALConditionVariable, GMALVisibleObject>(newMutexSharedPtr);
    }

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALCondInit(threadThatRan, condThatExists);
}

std::shared_ptr<GMALTransition>
GMALCondInit::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto condCpy =
            std::static_pointer_cast<GMALConditionVariable, GMALVisibleObject>(this->conditionVariable->copy());
    auto condInit = new GMALCondInit(threadCpy, condCpy);
    return std::shared_ptr<GMALTransition>(condInit);
}

std::shared_ptr<GMALTransition>
GMALCondInit::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALConditionVariable> mutexInState = state->getObjectWithId<GMALConditionVariable>(conditionVariable->getObjectId());
    auto cpy = new GMALCondInit(threadInState, mutexInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALCondInit::applyToState(GMALState *state)
{
    // TODO: Implement this
}

void
GMALCondInit::unapplyToState(GMALState *state)
{
    // TODO: Implement this
}

bool
GMALCondInit::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    return true;
}

bool
GMALCondInit::dependentWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeCondOperation = std::dynamic_pointer_cast<GMALCondTransition, GMALTransition>(other);
    if (maybeCondOperation) {
        return *maybeCondOperation->conditionVariable == *this->conditionVariable;
    }
    return false;
}

void
GMALCondInit::print()
{
    printf("thread %lu: pthread_cond_init(%lu)\n", this->thread->tid, this->conditionVariable->getObjectId());
}