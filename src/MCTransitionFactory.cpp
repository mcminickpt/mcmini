#include "MCTransitionFactory.h"
#include "transitions/threads/MCThreadCreate.h"
#include "transitions/threads/MCThreadJoin.h"

std::shared_ptr<MCTransition>
MCTransitionFactory::createInitialTransitionForThread(const std::shared_ptr<MCThread> &thread)
{
    return std::make_shared<MCThreadStart>(thread);
}

bool
MCTransitionFactory::transitionsCoenabledCommon(const MCTransition *t1, const MCTransition *t2)
{
    if (t1->getThreadId() == t2->getThreadId()) 
        return false;

    {
        const MCThreadCreate *maybeThreadCreate_t1 = dynamic_cast<const MCThreadCreate*>(t1);
        if (maybeThreadCreate_t1 != nullptr)
            return !maybeThreadCreate_t1->doesCreateThread(t2->getThreadId());

        const MCThreadCreate * maybeThreadCreate_t2 = dynamic_cast<const MCThreadCreate*>(t2);
        if (maybeThreadCreate_t2)
            return !maybeThreadCreate_t2->doesCreateThread(t1->getThreadId());
    }

    {
        const MCThreadJoin *maybeThreadJoin_t1 = dynamic_cast<const MCThreadJoin*>(t1);
        if (maybeThreadJoin_t1)
            return !maybeThreadJoin_t1->joinsOnThread(t2->getThreadId());

        const MCThreadJoin *maybeThreadJoin_t2 = dynamic_cast<const MCThreadJoin*>(t2);
        if (maybeThreadJoin_t2)
            return !maybeThreadJoin_t2->joinsOnThread(t1->getThreadId());
    }

    return true;
}

bool
MCTransitionFactory::transitionsDependentCommon(const MCTransition *t1, const MCTransition *t2)
{
    if (t1->getThreadId() == t2->getThreadId())
        return true;

    {
        const MCThreadCreate *maybeThreadCreate_t1 = dynamic_cast<const MCThreadCreate*>(t1);
        if (maybeThreadCreate_t1 != nullptr)
            return maybeThreadCreate_t1->doesCreateThread(t2->getThreadId());

        const MCThreadCreate * maybeThreadCreate_t2 = dynamic_cast<const MCThreadCreate*>(t2);
        if (maybeThreadCreate_t2)
            return maybeThreadCreate_t2->doesCreateThread(t1->getThreadId());
    }

    {
        const MCThreadJoin *maybeThreadJoin_t1 = dynamic_cast<const MCThreadJoin*>(t1);
        if (maybeThreadJoin_t1)
            return maybeThreadJoin_t1->joinsOnThread(t2->getThreadId());

        const MCThreadJoin *maybeThreadJoin_t2 = dynamic_cast<const MCThreadJoin*>(t2);
        if (maybeThreadJoin_t2)
            return maybeThreadJoin_t2->joinsOnThread(t1->getThreadId());
    }

    return false;
}
