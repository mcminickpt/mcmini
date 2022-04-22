#ifndef GMAL_GMALTHREADJOIN_H
#define GMAL_GMALTHREADJOIN_H

#include "GMALShared.h"
#include "GMALThreadTransition.h"

GMALTransition* GMALReadThreadJoin(const GMALSharedTransition*, void*, GMALState*);

struct GMALThreadJoin : public GMALThreadTransition {
public:
    inline GMALThreadJoin(std::shared_ptr<GMALThread> threadRunning, std::shared_ptr<GMALThread> joinedOn) :
            GMALThreadTransition(threadRunning, joinedOn) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    void unapplyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    bool enabledInState(const GMALState *) override;

    bool joinsOnThread(tid_t) const;
    bool joinsOnThread(const std::shared_ptr<GMALThread>&) const;

    void print() override;
};

#endif //GMAL_GMALTHREADJOIN_H
