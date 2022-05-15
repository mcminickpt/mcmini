#include "GMALBarrierInit.h"

GMALTransition*
GMALReadBarrierInit(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    auto barrierInShm = static_cast<GMALBarrierShadow*>(shmData);
    auto systemId = (GMALSystemID)barrierInShm->systemIdentity;
    auto barrierThatExists = state->getVisibleObjectWithSystemIdentity<GMALBarrier>(systemId);

    if (barrierThatExists == nullptr) {
        auto newBarrier = new GMALBarrier(*barrierInShm);
        auto newBarrierSharedPtr = std::shared_ptr<GMALVisibleObject>(newBarrier);
        state->registerVisibleObjectWithSystemIdentity(systemId, newBarrierSharedPtr);
        barrierThatExists = std::static_pointer_cast<GMALBarrier, GMALVisibleObject>(newBarrierSharedPtr);
    }

    tid_t threadThatRanId = shmTransition->executor;
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALBarrierInit(threadThatRan, barrierThatExists);
}

std::shared_ptr<GMALTransition>
GMALBarrierInit::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto mutexCpy =
            std::static_pointer_cast<GMALBarrier, GMALVisibleObject>(this->barrier->copy());
    auto mutexInit = new GMALBarrierInit(threadCpy, mutexCpy);
    return std::shared_ptr<GMALTransition>(mutexInit);
}

std::shared_ptr<GMALTransition>
GMALBarrierInit::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALBarrier> mutexInState = state->getObjectWithId<GMALBarrier>(barrier->getObjectId());
    auto cpy = new GMALBarrierInit(threadInState, mutexInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALBarrierInit::applyToState(GMALState *state)
{
    this->barrier->init();
}

bool
GMALBarrierInit::coenabledWith(std::shared_ptr<GMALTransition> other)
{
    return true;
}

bool
GMALBarrierInit::dependentWith(std::shared_ptr<GMALTransition> other)
{
    auto maybeBarrierOperation = std::dynamic_pointer_cast<GMALBarrierTransition, GMALTransition>(other);
    if (maybeBarrierOperation) {
        return *maybeBarrierOperation->barrier == *this->barrier;
    }
    return false;
}

void
GMALBarrierInit::print()
{
    printf("thread %lu: pthread_barrier_init(%lu, _, %u)\n", this->thread->tid, this->barrier->getObjectId(), this->barrier->getCount());
}