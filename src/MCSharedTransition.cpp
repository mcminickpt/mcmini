#include "mcmini/MCSharedTransition.h"
#include <cstring>

void
MCSharedTransitionReplace(MCSharedTransition *shmOld,
                          MCSharedTransition *shmNew)
{
  // NOTE: This cast could be done in a more complicated C++ way:
  //       https://stackoverflow.com/questions/66368061/error-clearing-an-object-of-non-trivial-type-with-memset
  memcpy((void *)shmOld, (void *)shmNew, sizeof(MCSharedTransition));
}
