#include "MCTransitionFactory.h"
#include "transitions/threads/MCThreadCreate.h"
#include "transitions/threads/MCThreadJoin.h"

std::shared_ptr<MCTransition>
MCTransitionFactory::createInitialTransitionForThread(const std::shared_ptr<MCThread> &thread)
{
    return std::make_shared<MCThreadStart>(thread);
}

bool
MCTransitionFactory::transitionsCoenabledCommon(const std::shared_ptr<MCTransition>& t1, const std::shared_ptr<MCTransition>& t2)
{
    if (t1->getThreadId() == t2->getThreadId()) {
        return false;
    }

    {
        auto maybeThreadCreate_t1 = std::dynamic_pointer_cast<MCThreadCreate, MCTransition>(t1);
        if (maybeThreadCreate_t1 != nullptr) {
            return !maybeThreadCreate_t1->doesCreateThread(t2->getThreadId());
        }

        auto maybeThreadCreate_t2 = std::dynamic_pointer_cast<MCThreadCreate, MCTransition>(t2);
        if (maybeThreadCreate_t2) {
            return !maybeThreadCreate_t2->doesCreateThread(t1->getThreadId());
        }
    }

    {
        auto maybeThreadJoin_t1 = std::dynamic_pointer_cast<MCThreadJoin, MCTransition>(t1);
        if (maybeThreadJoin_t1) {
            return !maybeThreadJoin_t1->joinsOnThread(t2->getThreadId());
        }

        auto maybeThreadJoin_t2 = std::dynamic_pointer_cast<MCThreadJoin, MCTransition>(t2);
        if (maybeThreadJoin_t2) {
            return !maybeThreadJoin_t2->joinsOnThread(t1->getThreadId());
        }
    }

    return true;
}

bool
MCTransitionFactory::transitionsDependentCommon(const std::shared_ptr<MCTransition> &t1, const std::shared_ptr<MCTransition> &t2)
{
    if (t1->getThreadId() == t2->getThreadId()) {
        return true;
    }

    {
        auto maybeThreadCreate_t1 = std::dynamic_pointer_cast<MCThreadCreate, MCTransition>(t1);
        if (maybeThreadCreate_t1 != nullptr) {
            return maybeThreadCreate_t1->doesCreateThread(t2->getThreadId());
        }

        auto maybeThreadCreate_t2 = std::dynamic_pointer_cast<MCThreadCreate, MCTransition>(t2);
        if (maybeThreadCreate_t2) {
            return maybeThreadCreate_t2->doesCreateThread(t1->getThreadId());
        }
    }

    {
        auto maybeThreadJoin_t1 = std::dynamic_pointer_cast<MCThreadJoin, MCTransition>(t1);
        if (maybeThreadJoin_t1) {
            return maybeThreadJoin_t1->joinsOnThread(t2->getThreadId());
        }

        auto maybeThreadJoin_t2 = std::dynamic_pointer_cast<MCThreadJoin, MCTransition>(t2);
        if (maybeThreadJoin_t2) {
            return maybeThreadJoin_t2->joinsOnThread(t1->getThreadId());
        }
    }

    return false;
}
