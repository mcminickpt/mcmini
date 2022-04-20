#ifndef GMAL_GMALTHREADCREATE_H
#define GMAL_GMALTHREADCREATE_H

#include "GMALShared.h"
#include "GMALThreadTransition.h"

GMALTransition* GMALReadThreadCreate(const GMALSharedTransition*, void*, GMALState*);

struct GMALThreadCreate : public GMALThreadTransition {
public:
    inline GMALThreadCreate(std::shared_ptr<GMALThread> threadRunning, std::shared_ptr<GMALThread> threadCreated) :
    GMALThreadTransition(threadRunning, threadCreated) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    void unapplyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;

    bool doesCreateThread(tid_t) const;
};

#endif //GMAL_GMALTHREADCREATE_H
