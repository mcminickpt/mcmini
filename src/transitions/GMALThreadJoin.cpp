#include "GMALThreadJoin.h"

GMALTransition*
GMALReadThreadJoin(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    // TODO: Potentially add asserts that the thread that just ran exists!
    auto newThreadData = static_cast<GMALThreadShadow *>(shmData);

    auto threadThatExists = state->getVisibleObjectWithSystemIdentity<GMALThread>((GMALSystemID)newThreadData->systemIdentity);
    tid_t newThreadId = threadThatExists != nullptr ? threadThatExists->tid : state->addNewThread(*newThreadData);
    tid_t threadThatRanId = shmTransition->executor;

    auto joinThread = state->getThreadWithId(newThreadId);
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALThreadJoin(threadThatRan, joinThread);
}

std::shared_ptr<GMALTransition>
GMALThreadJoin::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto targetThreadCpy =
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->target->copy());
    auto threadStartCpy = new GMALThreadJoin(threadCpy, targetThreadCpy);
    return std::shared_ptr<GMALTransition>(threadStartCpy);
}

std::shared_ptr<GMALTransition>
GMALThreadJoin::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALThread> targetInState = state->getThreadWithId(target->tid);
    auto cpy = new GMALThreadJoin(threadInState, targetInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

bool
GMALThreadJoin::enabledInState(const GMALState *) {
    return thread->enabled() && target->getState() == GMALThreadShadow::dead;
}

void
GMALThreadJoin::applyToState(GMALState *state)
{
    if (target->isDead()) {
        thread->awaken();
    } else {
        thread->sleep();
    }
}

void
GMALThreadJoin::unapplyToState(GMALState *state)
{
    thread->awaken();
}

bool
GMALThreadJoin::coenabledWith(std::shared_ptr<GMALTransition> transition)
{
    tid_t targetThreadId = transition->getThreadId();
    if (this->thread->tid == targetThreadId || this->target->tid == targetThreadId) {
        return false;
    }
    return true;
}

bool
GMALThreadJoin::dependentWith(std::shared_ptr<GMALTransition> transition)
{
    tid_t targetThreadId = transition->getThreadId();
    if (this->thread->tid == targetThreadId || this->target->tid == targetThreadId) {
        return true;
    }
    return false;
}