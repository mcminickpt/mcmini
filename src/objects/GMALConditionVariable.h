#ifndef GMAL_GMALCONDITIONVARIABLE_H
#define GMAL_GMALCONDITIONVARIABLE_H

#include "GMALVisibleObject.h"
#include "GMALMutex.h"
#include "misc/GMALOptional.h"
#include <vector>
#include <deque>

struct GMALSharedMemoryConditionVariable {
    pthread_cond_t *cond;
    pthread_mutex_t *mutex;

    GMALSharedMemoryConditionVariable(pthread_cond_t *cond, pthread_mutex_t *mutex)
    : cond(cond), mutex(mutex) {}
};

struct GMALConditionVariableShadow {
    pthread_cond_t *cond;
    enum GMALConditionVariableState {
        undefined, initialized, destroyed
    } state;
    GMALConditionVariableShadow(pthread_cond_t *cond) : cond(cond), state(undefined) {}
};

struct GMALConditionVariable : public GMALVisibleObject {
private:

    GMALConditionVariableShadow condShadow;

    /**
     * The collection of threads are currently asleep waiting
     * to be awoken by this queue
     */
    std::deque<tid_t> sleepQueue;
    std::deque<tid_t> wakeQueue;

    void removeSleepingThread(tid_t);
    void removeWakingThread(tid_t);
    bool threadIsInWaitingQueue(tid_t);

    inline explicit GMALConditionVariable(GMALConditionVariableShadow condShadow, std::shared_ptr<GMALMutex> mutex, objid_t id)
    : GMALVisibleObject(id), condShadow(condShadow), mutex(mutex) {}

public:

    // TODO: Figure out how to make this API a bit neater. For now
    // we avoid the best interface here to get condition variables to work

    // The lock that is used to gain access to this condition variable
    // This value may be NULL before the condition variable has received
    // a pthread_cond_wait call
    //
    // Note it is undefined to access a single condition variable with two
    // different locks
    std::shared_ptr<GMALMutex> mutex;

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

    bool isInitialized() const;
    bool isDestroyed() const;

    void initialize();
    void destroy();
    void enterSleepingQueue(tid_t);
    void wakeThread(tid_t);
    void wakeFirstThreadIfPossible();
    void wakeAllSleepingThreads();
    void removeThread(tid_t);
    bool threadCanExit(tid_t);
};

#endif //GMAL_GMALCONDITIONVARIABLE_H
