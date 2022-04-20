#include "GMALTransitionFactory.h"
#include "transitions/GMALThreadCreate.h"
#include "transitions/GMALThreadJoin.h"

std::shared_ptr<GMALTransition>
GMALTransitionFactory::createInitialTransitionForThread(std::shared_ptr<GMALThread> thread)
{
    auto tStart = new GMALThreadStart(thread);
    return std::shared_ptr<GMALTransition>(tStart);
}

bool
GMALTransitionFactory::transitionsCoenabledCommon(const std::shared_ptr<GMALTransition>& t1, const std::shared_ptr<GMALTransition>& t2)
{
    if (t1->getThreadId() == t2->getThreadId()) {
        return false;
    }

    {
        auto maybeThreadCreate_t1 = std::dynamic_pointer_cast<GMALThreadCreate, GMALTransition>(t1);
        if (maybeThreadCreate_t1 != nullptr) {
            return !maybeThreadCreate_t1->doesCreateThread(t2->getThreadId());
        }

        auto maybeThreadCreate_t2 = std::dynamic_pointer_cast<GMALThreadCreate, GMALTransition>(t2);
        if (maybeThreadCreate_t2) {
            return !maybeThreadCreate_t2->doesCreateThread(t1->getThreadId());
        }
    }

    {
        auto maybeThreadJoin_t1 = std::dynamic_pointer_cast<GMALThreadJoin, GMALTransition>(t1);
        if (maybeThreadJoin_t1) {
            return !maybeThreadJoin_t1->joinsOnThread(t2->getThreadId());
        }

        auto maybeThreadJoin_t2 = std::dynamic_pointer_cast<GMALThreadJoin, GMALTransition>(t2);
        if (maybeThreadJoin_t2) {
            return !maybeThreadJoin_t2->joinsOnThread(t1->getThreadId());
        }
    }

    return true;
}

bool
GMALTransitionFactory::transitionsDependentCommon(const std::shared_ptr<GMALTransition> &t1, const std::shared_ptr<GMALTransition> &t2)
{
    if (t1->getThreadId() == t2->getThreadId()) {
        return true;
    }

    {
        auto maybeThreadCreate_t1 = std::dynamic_pointer_cast<GMALThreadCreate, GMALTransition>(t1);
        if (maybeThreadCreate_t1 != nullptr) {
            return maybeThreadCreate_t1->doesCreateThread(t2->getThreadId());
        }

        auto maybeThreadCreate_t2 = std::dynamic_pointer_cast<GMALThreadCreate, GMALTransition>(t2);
        if (maybeThreadCreate_t2) {
            return maybeThreadCreate_t2->doesCreateThread(t1->getThreadId());
        }
    }

    {
        auto maybeThreadJoin_t1 = std::dynamic_pointer_cast<GMALThreadJoin, GMALTransition>(t1);
        if (maybeThreadJoin_t1) {
            return maybeThreadJoin_t1->joinsOnThread(t2->getThreadId());
        }

        auto maybeThreadJoin_t2 = std::dynamic_pointer_cast<GMALThreadJoin, GMALTransition>(t2);
        if (maybeThreadJoin_t2) {
            return maybeThreadJoin_t2->joinsOnThread(t1->getThreadId());
        }
    }

    return false;
}

