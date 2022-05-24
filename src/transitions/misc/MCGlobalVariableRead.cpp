#include "MCGlobalVariableRead.h"
#include "MCGlobalVariableWrite.h"

MCTransition*
MCReadGlobalRead(const MCSharedTransition *shmTransition, void *shmStart, MCState *state)
{
    auto addr = *(void**)shmStart;
    auto threadThatRan = state->getThreadWithId(shmTransition->executor);
    auto globalVariable = state->getVisibleObjectWithSystemIdentity<MCGlobalVariable>(addr);

    /* New global variable */
    if (globalVariable == nullptr) {
        globalVariable = std::make_shared<MCGlobalVariable>(addr);
        state->registerVisibleObjectWithSystemIdentity(addr, globalVariable);
    }

    return new MCGlobalVariableRead(threadThatRan, globalVariable);
}

std::shared_ptr<MCTransition>
MCGlobalVariableRead::staticCopy() {
    auto threadCpy=
            std::static_pointer_cast<MCThread, MCVisibleObject>(this->thread->copy());
    auto globalCpy = std::static_pointer_cast<MCGlobalVariable, MCVisibleObject>(this->global->copy());
    return std::make_shared<MCGlobalVariableRead>(threadCpy, globalCpy);
}

std::shared_ptr<MCTransition>
MCGlobalVariableRead::dynamicCopyInState(const MCState *state) {
    std::shared_ptr<MCThread> threadInState = state->getThreadWithId(thread->tid);
    auto globalInState = state->getObjectWithId<MCGlobalVariable>(global->getObjectId());
    return std::make_shared<MCGlobalVariableRead>(threadInState, globalInState);
}

bool
MCGlobalVariableRead::coenabledWith(std::shared_ptr<MCTransition>)
{
    return true; /* Co-enabled with anything else */
}

bool
MCGlobalVariableRead::dependentWith(std::shared_ptr<MCTransition> transition)
{
    auto maybeGlobalVariableWrite = std::dynamic_pointer_cast<MCGlobalVariableWrite, MCTransition>(transition);
    if (maybeGlobalVariableWrite != nullptr) {
        return *this->global == *maybeGlobalVariableWrite->global;
    }
    return false;
}

bool
MCGlobalVariableRead::isRacingWith(std::shared_ptr<MCTransition> transition)
{
    auto maybeGlobalVariableWrite = std::dynamic_pointer_cast<MCGlobalVariableWrite, MCTransition>(transition);
    if (maybeGlobalVariableWrite != nullptr) {
        return *this->global == *maybeGlobalVariableWrite->global;
    }
    return false;
}

void
MCGlobalVariableRead::print()
{
    printf("thread %lu: READ(%p)\n", this->thread->tid, this->global->addr);
}
