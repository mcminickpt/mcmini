#include "GMALMutexInit.h"

GMALTransition*
GMALReadMutexInit(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    auto mutexInShm = static_cast<GMALMutexShadow*>(shmData);
    auto systemId = (GMALSystemID)mutexInShm->systemIdentity;
    auto mutexThatExists = state->getVisibleObjectWithSystemIdentity<GMALMutex>(systemId);

    if (mutexThatExists == nullptr) {
        auto newMutex = new GMALMutex(*mutexInShm);
        auto newMutexSharedPtr = std::shared_ptr<GMALVisibleObject>(newMutex);
        state->registerVisibleObjectWithSystemIdentity(systemId, newMutexSharedPtr);
        mutexThatExists = std::static_pointer_cast<GMALMutex, GMALVisibleObject>(newMutexSharedPtr);
    }

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALMutexInit(threadThatRan, mutexThatExists);
}

std::shared_ptr<GMALTransition>
GMALMutexInit::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto mutexCpy =
            std::static_pointer_cast<GMALMutex, GMALVisibleObject>(this->mutex->copy());
    auto mutexInit = new GMALMutexInit(threadCpy, mutexCpy);
    return std::shared_ptr<GMALTransition>(mutexInit);
}

std::shared_ptr<GMALTransition>
GMALMutexInit::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALMutex> mutexInState = state->getObjectWithId<GMALMutex>(mutex->getObjectId());
    auto cpy = new GMALMutexInit(threadInState, mutexInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALMutexInit::applyToState(GMALState *state)
{
    this->mutex->init();
}

void
GMALMutexInit::unapplyToState(GMALState *state)
{
    this->mutex->deinit();
}

bool
GMALMutexInit::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    return true;
}

bool
GMALMutexInit::dependentWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeMutexOperation = std::dynamic_pointer_cast<GMALMutexTransition, GMALTransition>(other);
    if (maybeMutexOperation) {
        return *maybeMutexOperation->mutex == *this->mutex;
    }
    return false;
}

void
GMALMutexInit::print()
{
    puts("************************");
    puts(" -- MUTEX INIT -- ");
    this->thread->print();
    this->mutex->print();
    puts("************************");
}