//
// Created by parallels on 4/14/22.
//

#include "GMALThreadCreate.h"

GMALTransition*
GMALReadThreadCreate(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state)
{
    // TODO: Potentially add asserts that the thread that just ran exists!
    auto newThreadData = static_cast<GMALThreadShadow *>(shmData);

    auto threadThatExists = state->getVisibleObjectWithSystemIdentity<GMALThread>((GMALSystemID)newThreadData->systemIdentity);
    tid_t newThreadId = threadThatExists != nullptr ? threadThatExists->tid : state->addNewThread(*newThreadData);
    tid_t threadThatRanId = shmTransition->executor;

    auto newThread = state->getThreadWithId(newThreadId);
    auto threadThatRan = state->getThreadWithId(threadThatRanId);
    return new GMALThreadCreate(threadThatRan, newThread);
}

std::shared_ptr<GMALTransition>
GMALThreadCreate::staticCopy()
{
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto targetThreadCpy =
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->target->copy());
    auto threadStartCpy = new GMALThreadCreate(threadCpy, targetThreadCpy);
    return std::shared_ptr<GMALTransition>(threadStartCpy);
}

std::shared_ptr<GMALTransition>
GMALThreadCreate::dynamicCopyInState(const GMALState *state)
{
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    std::shared_ptr<GMALThread> targetInState = state->getThreadWithId(target->tid);
    auto cpy = new GMALThreadCreate(threadInState, targetInState);
    return std::shared_ptr<GMALTransition>(cpy);
}

void
GMALThreadCreate::applyToState(GMALState *state)
{
    this->target->spawn();
}

void
GMALThreadCreate::unapplyToState(GMALState *state)
{
    this->target->despawn();
}
