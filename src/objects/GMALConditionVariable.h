#ifndef MC_MCCONDITIONVARIABLE_H
#define MC_MCCONDITIONVARIABLE_H

#include "MCVisibleObject.h"
#include "MCMutex.h"
#include "misc/MCOptional.h"
#include <vector>
#include <deque>

struct MCSharedMemoryConditionVariable {
    pthread_cond_t *cond;
    pthread_mutex_t *mutex;

    MCSharedMemoryConditionVariable(pthread_cond_t *cond, pthread_mutex_t *mutex)
    : cond(cond), mutex(mutex) {}
};

struct MCConditionVariableShadow {
    pthread_cond_t *cond;
    enum MCConditionVariableState {
        undefined, initialized, destroyed
    } state;
    MCConditionVariableShadow(pthread_cond_t *cond) : cond(cond), state(undefined) {}
};

struct MCConditionVariable : public MCVisibleObject {
private:

    MCConditionVariableShadow condShadow;

    /**
     * The collection of threads are currently asleep waiting
     * to be awoken by this queue
     */
    std::deque<tid_t> sleepQueue;
    std::deque<tid_t> wakeQueue;

    void removeSleepingThread(tid_t);
    void removeWakingThread(tid_t);
    bool threadIsInWaitingQueue(tid_t);

    inline explicit MCConditionVariable(MCConditionVariableShadow condShadow, std::shared_ptr<MCMutex> mutex, objid_t id)
    : MCVisibleObject(id), condShadow(condShadow), mutex(mutex) {}

public:

    // TODO: Figure out how to make this API a bit neater. For now
    // we avoid the best interface here to get condition variables to work

    // The lock that is used to gain access to this condition variable
    // This value may be NULL before the condition variable has received
    // a pthread_cond_wait call
    //
    // Note it is undefined to access a single condition variable with two
    // different locks
    std::shared_ptr<MCMutex> mutex;

    inline explicit MCConditionVariable(MCConditionVariableShadow condShadow, std::shared_ptr<MCMutex> mutex)
    : MCVisibleObject(), condShadow(condShadow), mutex(mutex) {}

    inline explicit MCConditionVariable(MCConditionVariableShadow condShadow)
            : MCVisibleObject(), condShadow(condShadow), mutex(nullptr) {}

    inline MCConditionVariable(const MCConditionVariable &cond)
    : MCVisibleObject(cond.getObjectId()), condShadow(cond.condShadow), mutex(nullptr) {

        if (cond.mutex != nullptr) {
            mutex = std::static_pointer_cast<MCMutex, MCVisibleObject>(cond.mutex->copy());
        }
    }

    std::shared_ptr<MCVisibleObject> copy() override;
    MCSystemID getSystemId() override;


    bool operator ==(const MCConditionVariable&) const;
    bool operator !=(const MCConditionVariable&) const;

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

#endif //MC_MCCONDITIONVARIABLE_H
