#include "MCSharedTransition.h"

void
MCSharedTransitionReplace(MCSharedTransition *shmOld, MCSharedTransition *shmNew)
{
    memcpy(shmOld, shmNew, sizeof(MCSharedTransition));
}