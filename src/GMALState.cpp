#include "GMALState.h"

std::shared_ptr<GMALTransition>
GMALState::getSlotForThread(GMALThread *thread)
{
    return nextTransitions[thread->tid];
}

std::shared_ptr<GMALTransition>
GMALState::getSlotForThread(tid_t tid)
{
    return nextTransitions[tid];
}

objid_t
GMALState::registerNewObject(GMALVisibleObject *object)
{
    return objectStorage.registerNewObject(object);
}

objid_t
GMALState::softRegisterNewObject(GMALVisibleObject *object)
{
    return objectStorage.registerNewObject(object);
}

template<typename Object>
std::shared_ptr<Object>
GMALState::getObjectWithId(objid_t id) const
{
    return objectStorage.getObjectWithId<Object>(id);
}

std::shared_ptr<GMALThread>
GMALState::getThreadWithId(tid_t id) const
{
    return objectStorage.getObjectWithId<GMALThread>(id);
}

void
GMALState::setNextTransitionForThread(GMALThread *thread, std::shared_ptr<GMALTransition> transition)
{
    this->setNextTransitionForThread(thread->tid, transition);
}

void
GMALState::setNextTransitionForThread(tid_t tid, std::shared_ptr<GMALTransition> transition)
{
    this->nextTransitions[tid] = transition;
}

void
GMALState::setNextTransitionForThread(tid_t tid, GMALSharedTransition *shmData)
{
    GMALSharedMemoryHandler handlerForType = this->sharedMemoryHandlerTypeMap.find(shmData->type)->second;
    GMALTransition *newTransitionForThread = handlerForType(&shmData->data, *this);
    auto sharedPointer = std::shared_ptr<GMALTransition>(newTransitionForThread);
    this->setNextTransitionForThread(tid, sharedPointer);
}

tid_t
GMALState::createNewThread()
{
    tid_t newTid = this->nextThreadId++;
    auto thread = new GMALThread(newTid, nullptr, nullptr, pthread_self());
    objid_t objid = this->registerNewObject(thread);
    return newTid;
}

tid_t
GMALState::createMainThread()
{
    tid_t mainThreadId = this->createNewThread();
    GMAL_ASSERT(mainThreadId == TID_MAIN_THREAD);
    return mainThreadId;
}

uint64_t
GMALState::getTransitionStackSize() const
{
    if (this->t_stack_top < 0) return 0;
    return this->t_stack_top + 1;
}

uint64_t
GMALState::getStateStackSize() const
{
    if (this->s_stack_top < 0) return 0;
    return this->s_stack_top + 1;
}

void
GMALState::registerVisibleOperationType(GMALType type, GMALSharedMemoryHandler handler)
{
    this->sharedMemoryHandlerTypeMap.insert({type, handler});
}