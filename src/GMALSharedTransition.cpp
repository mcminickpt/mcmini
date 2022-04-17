//
// Created by parallels on 4/10/22.
//

#include "GMALSharedTransition.h"

void
GMALSharedTransitionReplace(GMALSharedTransition *shmOld, GMALSharedTransition *shmNew)
{
    memcpy(shmOld, shmNew, sizeof(GMALSharedTransition));
}