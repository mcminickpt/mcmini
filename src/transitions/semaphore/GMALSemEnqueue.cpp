#include "GMALSemEnqueue.h"
#include "GMAL.h"

GMALTransition*
GMALReadSemEnqueue(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    auto semInShm = *static_cast<sem_t**>(shmData);
    auto semThatExists = state->getVisibleObjectWithSystemIdentity<GMALSemaphore>((GMALSystemID)semInShm);

    // Catch undefined behavior
    GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(semThatExists != nullptr, "Attempting to wait on an uninitialized semaphore");
    if (semThatExists->isDestroyed()) {
        GMAL_REPORT_UNDEFINED_BEHAVIOR("Attempting to wait on a semaphore that has been destroyed");
    }

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALSemEnqueue(threadThatRan, semThatExists);
}

std::shared_ptr<GMALTransition>
GMALSemEnqueue::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto semCpy =
            std::static_pointer_cast<GMALSemaphore, GMALVisibleObject>(this->sem->copy());
    auto mutexInit = new GMALSemEnqueue(threadCpy, semCpy);
    return std::shared_ptr<GMALTransition>(mutexInit);
}

std::shared_ptr<GMALTransition>
GMALSemEnqueue::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALSemaphore> semInState = state->getObjectWithId<GMALSemaphore>(sem->getObjectId());
    auto cpy = new GMALSemEnqueue(threadInState, semInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALSemEnqueue::applyToState(GMALState *state)
{
    this->sem->enterWaitingQueue(this->getThreadId());
}

bool
GMALSemEnqueue::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    return true;
}

bool
GMALSemEnqueue::dependentWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeSemaphoreOperation = std::dynamic_pointer_cast<GMALSemaphoreTransition, GMALTransition>(other);
    if (maybeSemaphoreOperation) {
        return *maybeSemaphoreOperation->sem == *this->sem;
    }
    return false;
}

void
GMALSemEnqueue::print()
{
    printf("thread %lu: sem_wait(%lu) (enter)\n", this->thread->tid, this->sem->getObjectId());
}


