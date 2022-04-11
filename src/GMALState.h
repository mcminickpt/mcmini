#ifndef GMAL_GMALSTATE_H
#define GMAL_GMALSTATE_H

struct GMALTransition;

#include "GMALShared.h"
#include "GMALMap.h"
#include "GMALTransition.h"
#include "GMALObjectStore.h"
#include "GMALRef.h"
#include "GMALThread.h"

class GMALState {
private:
    GMALObjectStore objectStorage;

    tid_t nextThreadId = 0;
    // Maps thread ids to what they run next
    std::shared_ptr<GMALTransition> nextTransitions[MAX_TOTAL_THREADS_IN_PROGRAM];
public:
    std::shared_ptr<GMALTransition> getSlotForThread(GMALRef<GMALThread> thread);
    std::shared_ptr<GMALTransition> getSlotForThread(GMALThread *thread);
    std::shared_ptr<GMALTransition> getSlotForThread(tid_t thread);

    void setNextTransitionForThread(GMALRef<GMALThread>, GMALTransition*);
    void setNextTransitionForThread(GMALThread *, GMALTransition*);
    void setNextTransitionForThread(tid_t, GMALTransition*);

    objid_t registerNewObject(GMALVisibleObject *object);
    objid_t softRegisterNewObject(GMALVisibleObject *object);

    std::shared_ptr<GMALVisibleObject> getObjectWithId(objid_t id);
    std::shared_ptr<GMALVisibleObject> getThreadWithId(tid_t id);
    tid_t createNewThread();
    tid_t createMainThread();
};

#endif //GMAL_GMALSTATE_H
