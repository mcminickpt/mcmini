#ifndef GMAL_GMALCONDITIONVARIABLE_H
#define GMAL_GMALCONDITIONVARIABLE_H

#include "GMALVisibleObject.h"
#include "GMALMutex.h"
#include <vector>

struct GMALConditionVariableShadow {
    pthread_cond_t *cond;
    enum GMALConditionVariableState {
        undefined, initialized, destroyed
    } state;
    GMALConditionVariableShadow(pthread_cond_t *cond) : cond(cond), state(undefined) {}
};

struct GMALConditionVariable : public GMALVisibleObject {
private:
    // The lock that is used to gain access to this condition variable
    // This value may be NULL before the condition variable has received
    // a pthread_cond_wait call
    //
    // Note it is undefined to access a single condition variable with two
    // different locks
    std::shared_ptr<GMALMutex> mutex;

    GMALConditionVariableShadow condShadow;

    // TODO: There is at some difficulty
    // with condition variables in that to be able to
    // undo e.g. a pthread_cond_broadcast(), we'd need to
    // know the set of threads that that broadcast had awoken
    // since it may blend in with the set of threads that *were*
    // waiting before the broadcast. We couldn't simply put all
    // of the waiting threads to sleep as this might not be
    // equivalent to the state before calling broadcast()
    /**
     * The collection of threads are currently asleep waiting
     * to be awoken by this queue
     */
    std::vector<tid_t> sleepQueue;
    std::vector<tid_t> wakeQueue;

    inline explicit GMALConditionVariable(GMALConditionVariableShadow condShadow, std::shared_ptr<GMALMutex> mutex, objid_t id)
    : GMALVisibleObject(id), condShadow(condShadow), mutex(mutex) {}

public:
    inline explicit GMALConditionVariable(GMALConditionVariableShadow condShadow, std::shared_ptr<GMALMutex> mutex)
    : GMALVisibleObject(), condShadow(condShadow), mutex(mutex) {}

    inline explicit GMALConditionVariable(GMALConditionVariableShadow condShadow)
            : GMALVisibleObject(), condShadow(condShadow), mutex(nullptr) {}

    inline GMALConditionVariable(const GMALConditionVariable &cond)
    : GMALVisibleObject(cond.getObjectId()), condShadow(cond.condShadow), mutex(cond.mutex) {}

    std::shared_ptr<GMALVisibleObject> copy() override;
    GMALSystemID getSystemId() override;

    bool operator ==(const GMALConditionVariable&) const;
    bool operator !=(const GMALConditionVariable&) const;

    bool isOwned() const;
    bool isDestroyed() const;

    void relinquish();
    void takeOwnership();

    void print() override;
};

#endif //GMAL_GMALCONDITIONVARIABLE_H
