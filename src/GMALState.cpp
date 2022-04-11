#include "GMALState.h"

std::shared_ptr<GMALTransition>
GMALState::getSlotForThread(GMALRef<GMALThread> thread)
{
    return nextTransitions[thread->tid];
}

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

std::shared_ptr<GMALVisibleObject>
GMALState::getObjectWithId(objid_t id)
{
    return objectStorage.getObjectWithId(id);
}

std::shared_ptr<GMALVisibleObject>
GMALState::getThreadWithId(tid_t id)
{
    return objectStorage.getObjectWithId(0);
}

void
GMALState::setNextTransitionForThread(GMALRef<GMALThread> thread, GMALTransition *transition)
{
    this->setNextTransitionForThread(thread->tid, transition);
}

void
GMALState::setNextTransitionForThread(GMALThread *thread, GMALTransition *transition)
{
    this->setNextTransitionForThread(thread->tid, transition);
}

void
GMALState::setNextTransitionForThread(tid_t tid, GMALTransition *transition)
{
    this->nextTransitions[tid] = std::shared_ptr<GMALTransition>(transition);
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