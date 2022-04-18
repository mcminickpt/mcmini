#ifndef GMAL_GMALTHREADSTART_H
#define GMAL_GMALTHREADSTART_H

#include "GMALShared.h"
#include "GMALThreadTransition.h"

GMALTransition* GMALReadThreadStart(const GMALSharedTransition*, void*, GMALState*);

struct GMALThreadStart : public GMALThreadTransition {
public:
    inline explicit GMALThreadStart(std::shared_ptr<GMALThread> thread) : GMALThreadTransition(thread) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    void unapplyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;

};

#endif //GMAL_GMALTHREADSTART_H
