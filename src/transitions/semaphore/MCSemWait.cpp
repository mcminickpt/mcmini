#include "MCMINI.h"
#include "MCSemWait.h"
#include "MCSemInit.h"

MCTransition*
MCReadSemWait(const MCSharedTransition *shmTransition, void *shmData, MCState *state)
{
    auto semInShm = *static_cast<sem_t**>(shmData);
    auto semThatExists = state->getVisibleObjectWithSystemIdentity<MCSemaphore>((MCSystemID)semInShm);

    // Catch undefined behavior
    MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(semThatExists != nullptr, "Attempting to wait on an uninitialized semaphore");
    if (semThatExists->isDestroyed()) {
        MC_REPORT_UNDEFINED_BEHAVIOR("Attempting to wait on a semaphore that has been destroyed");
    }

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new MCSemWait(threadThatRan, semThatExists);
}

std::shared_ptr<MCTransition>
MCSemWait::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<MCThread, MCVisibleObject>(this->thread->copy());
    auto semCpy =
            std::static_pointer_cast<MCSemaphore, MCVisibleObject>(this->sem->copy());
    auto mutexInit = new MCSemWait(threadCpy, semCpy);
    return std::shared_ptr<MCTransition>(mutexInit);
}

std::shared_ptr<MCTransition>
MCSemWait::dynamicCopyInState(const MCState *state)
{
    std::shared_ptr<MCThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<MCSemaphore> semInState = state->getObjectWithId<MCSemaphore>(sem->getObjectId());
    auto cpy = new MCSemWait(threadInState, semInState);
    return std::shared_ptr<MCTransition>(cpy);
}

void
MCSemWait::applyToState(MCState *state)
{
    this->sem->wait();
    this->sem->leaveWaitingQueue(this->getThreadId());
}

bool
MCSemWait::coenabledWith(std::shared_ptr<MCTransition> other)
{
    return true;
}

bool
MCSemWait::dependentWith(std::shared_ptr<MCTransition> other)
{
    auto maybeSemaphoreInitOperation = std::dynamic_pointer_cast<MCSemInit, MCTransition>(other);
    if (maybeSemaphoreInitOperation) {
        return *maybeSemaphoreInitOperation->sem == *this->sem;
    }

    auto maybeSemaphoreWaitOperation = std::dynamic_pointer_cast<MCSemWait, MCTransition>(other);
    if (maybeSemaphoreWaitOperation) {
        return *maybeSemaphoreWaitOperation->sem == *this->sem;
    }

    return false;
}

bool
MCSemWait::enabledInState(const MCState *)
{
    return this->sem->threadCanExit(this->getThreadId());
}

void
MCSemWait::print()
{
    printf("thread %lu: sem_wait(%lu) (attempt exit)\n", this->thread->tid, this->sem->getObjectId());
}


