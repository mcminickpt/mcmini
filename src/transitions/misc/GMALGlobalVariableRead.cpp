#include "GMALGlobalVariableRead.h"
#include "GMALGlobalVariableWrite.h"

GMALTransition*
GMALReadGlobalRead(const GMALSharedTransition *shmTransition, void *shmStart, GMALState *state)
{
    auto addr = *(void**)shmStart;
    auto threadThatRan = state->getThreadWithId(shmTransition->executor);
    auto globalVariable = state->getVisibleObjectWithSystemIdentity<GMALGlobalVariable>(addr);

    /* New global variable */
    if (globalVariable == nullptr)
        globalVariable = std::make_shared<GMALGlobalVariable>(addr);

    return new GMALGlobalVariableRead(threadThatRan, globalVariable);
}

std::shared_ptr<GMALTransition>
GMALGlobalVariableRead::staticCopy() {
    auto threadCpy=
            std::static_pointer_cast<GMALThread, GMALVisibleObject>(this->thread->copy());
    auto globalCpy = std::static_pointer_cast<GMALGlobalVariable, GMALVisibleObject>(this->global->copy());
    return std::make_shared<GMALGlobalVariableRead>(threadCpy, globalCpy);
}

std::shared_ptr<GMALTransition>
GMALGlobalVariableRead::dynamicCopyInState(const GMALState *state) {
    std::shared_ptr<GMALThread> threadInState = state->getThreadWithId(thread->tid);
    auto globalInState = state->getObjectWithId<GMALGlobalVariable>(global->getObjectId());
    return std::make_shared<GMALGlobalVariableRead>(threadInState, globalInState);
}

bool
GMALGlobalVariableRead::coenabledWith(std::shared_ptr<GMALTransition>)
{
    return true; /* Co-enabled with anything else */
}

bool
GMALGlobalVariableRead::dependentWith(std::shared_ptr<GMALTransition> transition)
{
    auto maybeGlobalVariableWrite = std::dynamic_pointer_cast<GMALGlobalVariableWrite, GMALTransition>(transition);
    if (maybeGlobalVariableWrite != nullptr) {
        return *this->global == *maybeGlobalVariableWrite->global;
    }
    return false;
}

void
GMALGlobalVariableRead::print()
{
    printf("thread %lu: read_global_var(%p)\n", this->thread->tid, this->global->addr);
}
