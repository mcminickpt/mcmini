#include "GMALSemPost.h"

GMALTransition*
GMALReadSemPost(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    auto semInShm = static_cast<GMALSemaphoreShadow*>(shmData);
    auto semThatExists = state->getVisibleObjectWithSystemIdentity<GMALSemaphore>((GMALSystemID)semInShm->sem);

    // TODO: Figure out how to deal with undefined behavior
    GMAL_ASSERT(semThatExists != nullptr);

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALSemPost(threadThatRan, semThatExists);
}

std::shared_ptr<GMALTransition>
GMALSemPost::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto semCpy =
            std::static_pointer_cast<GMALSemaphore, GMALVisibleObject>(this->sem->copy());
    auto mutexInit = new GMALSemPost(threadCpy, semCpy);
    return std::shared_ptr<GMALTransition>(mutexInit);
}

std::shared_ptr<GMALTransition>
GMALSemPost::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALSemaphore> semInState = state->getObjectWithId<GMALSemaphore>(sem->getObjectId());
    auto cpy = new GMALSemPost(threadInState, semInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALSemPost::applyToState(GMALState *state)
{
    this->sem->post();
}

void
GMALSemPost::unapplyToState(GMALState *state)
{
    this->sem->wait();
}

bool
GMALSemPost::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    return true;
}

bool
GMALSemPost::dependentWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeSemaphoreOperation = std::dynamic_pointer_cast<GMALSemaphoreTransition, GMALTransition>(other);
    if (maybeSemaphoreOperation) {
        return *maybeSemaphoreOperation->sem == *this->sem;
    }
    return false;
}

void
GMALSemPost::print()
{
    puts("************************");
    puts(" -- SEM POST -- ");
    this->sem->print();
    puts("************************");
}