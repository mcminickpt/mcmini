#include "GMALBarrierEnqueue.h"
#include "GMAL.h"

GMALTransition*
GMALReadBarrierEnqueue(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    auto barrierInShm = static_cast<GMALBarrierShadow*>(shmData);
    auto barrierThatExists = state->getVisibleObjectWithSystemIdentity<GMALBarrier>((GMALSystemID)barrierInShm->systemIdentity);

    GMAL_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(barrierThatExists != nullptr, "Attempting to wait on a barrier that hasn't been initialized");

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALBarrierEnqueue(threadThatRan, barrierThatExists);
}

std::shared_ptr<GMALTransition>
GMALBarrierEnqueue::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto barrierCpy =
            std::static_pointer_cast<GMALBarrier, GMALVisibleObject>(this->barrier->copy());
    auto cpy = new GMALBarrierEnqueue(threadCpy, barrierCpy);
    return std::shared_ptr<GMALTransition>(cpy);
}

std::shared_ptr<GMALTransition>
GMALBarrierEnqueue::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALBarrier> barrierInState = state->getObjectWithId<GMALBarrier>(barrier->getObjectId());
    auto cpy = new GMALBarrierEnqueue(threadInState, barrierInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALBarrierEnqueue::applyToState(GMALState *state)
{
    auto executor = this->getThreadId();
    barrier->wait(executor); // Add this thread to the waiting queue -> potentially unblocks threads waiting on the barrier
}

bool
GMALBarrierEnqueue::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    return true;
}

bool
GMALBarrierEnqueue::dependentWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeBarrierOperation = std::dynamic_pointer_cast<GMALBarrierTransition, GMALTransition>(other);
    if (maybeBarrierOperation) {
        return *maybeBarrierOperation->barrier == *this->barrier;
    }
    return false;
}

void
GMALBarrierEnqueue::print()
{
    printf("thread %lu: pthread_barrier_wait(%lu) (enqueue)\n", this->thread->tid, this->barrier->getObjectId());
}


