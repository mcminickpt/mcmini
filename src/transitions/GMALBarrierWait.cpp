#include "GMALBarrierWait.h"

GMALTransition*
GMALReadBarrierWait(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    auto barrierInShm = static_cast<GMALBarrierShadow*>(shmData);
    auto barrierThatExists = state->getVisibleObjectWithSystemIdentity<GMALBarrier>((GMALSystemID)barrierInShm->systemIdentity);

    // TODO: Figure out how to deal with undefined behavior
    GMAL_ASSERT(barrierThatExists != nullptr);

    auto executor = shmTransition->executor;
    if (!barrierThatExists->isWaitingOnBarrier(executor)) {
        barrierThatExists->wait(executor); // Add this thread to the waiting queue -> potentially unblocks threads waiting on the barrier
    }

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALBarrierWait(threadThatRan, barrierThatExists);
}

std::shared_ptr<GMALTransition>
GMALBarrierWait::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto barrierCpy =
            std::static_pointer_cast<GMALBarrier, GMALVisibleObject>(this->barrier->copy());
    auto cpy = new GMALBarrierWait(threadCpy, barrierCpy);
    return std::shared_ptr<GMALTransition>(cpy);
}

std::shared_ptr<GMALTransition>
GMALBarrierWait::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALBarrier> barrierInState = state->getObjectWithId<GMALBarrier>(barrier->getObjectId());
    auto cpy = new GMALBarrierWait(threadInState, barrierInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALBarrierWait::applyToState(GMALState *state)
{
    // We don't actually need to do anything here
}


bool
GMALBarrierWait::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    return true;
}

bool
GMALBarrierWait::dependentWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeBarrierOperation = std::dynamic_pointer_cast<GMALBarrierTransition, GMALTransition>(other);
    if (maybeBarrierOperation) {
        return *maybeBarrierOperation->barrier == *this->barrier;
    }
    return false;
}

bool
GMALBarrierWait::enabledInState(const GMALState *state)
{
    return !this->barrier->wouldBlockIfWaitedOn(this->getThreadId());
}

void
GMALBarrierWait::print()
{
    printf("thread %lu: pthread_barrier_wait(%lu)\n", this->thread->tid, this->barrier->getObjectId());
}


