#include "GMAL.h"
#include "GMALSemWait.h"

GMALTransition*
GMALReadSemWait(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    auto semInShm = static_cast<GMALSemaphoreShadow*>(shmData);
    auto semThatExists = state->getVisibleObjectWithSystemIdentity<GMALSemaphore>((GMALSystemID)semInShm->sem);

    // Catch undefined behavior
    GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(semThatExists != nullptr, "Attempting to wait on an uninitialized semaphore");
    if (semThatExists->isDestroyed()) {
        GMAL_REPORT_UNDEFINED_BEHAVIOR("Attempting to wait on a semaphore that has been destroyed");
    }

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALSemWait(threadThatRan, semThatExists);
}

std::shared_ptr<GMALTransition>
GMALSemWait::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto semCpy =
            std::static_pointer_cast<GMALSemaphore, GMALVisibleObject>(this->sem->copy());
    auto mutexInit = new GMALSemWait(threadCpy, semCpy);
    return std::shared_ptr<GMALTransition>(mutexInit);
}

std::shared_ptr<GMALTransition>
GMALSemWait::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALSemaphore> semInState = state->getObjectWithId<GMALSemaphore>(sem->getObjectId());
    auto cpy = new GMALSemWait(threadInState, semInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALSemWait::applyToState(GMALState *state)
{
    this->sem->wait();
}

void
GMALSemWait::unapplyToState(GMALState *state)
{
    this->sem->post();
}

bool
GMALSemWait::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    return true;
}

bool
GMALSemWait::dependentWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeSemaphoreOperation = std::dynamic_pointer_cast<GMALSemaphoreTransition, GMALTransition>(other);
    if (maybeSemaphoreOperation) {
        return *maybeSemaphoreOperation->sem == *this->sem;
    }
    return false;
}

bool
GMALSemWait::enabledInState(const GMALState *)
{
    return !this->sem->wouldBlockIfWaitedOn();
}

void
GMALSemWait::print()
{
    printf("thread %lu: sem_wait(%lu)\n", this->thread->tid, this->sem->getObjectId());
}


