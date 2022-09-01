#include "mcmini/MCSharedTransition.h"
#include <cstring>

void
MCSharedTransitionReplace(MCSharedTransition *shmOld,
                          MCSharedTransition *shmNew)
{
  memcpy(shmOld, shmNew, sizeof(MCSharedTransition));
}