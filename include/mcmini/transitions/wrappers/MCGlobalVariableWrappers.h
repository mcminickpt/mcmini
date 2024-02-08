#ifndef MC_MCGLOBALVARIABLEWRAPPERS_H
#define MC_MCGLOBALVARIABLEWRAPPERS_H

#include "mcmini/MCShared.h"

MC_EXTERN void *mcmini_read(void *);
MC_EXTERN void mcmini_write(void *, void *);

#define MC_TRACK_READ(x)         mcmini_read(x)
#define MC_TRACK_WRITE(x, x_new) mcmini_write(x, x_new)

#endif // MC_MCGLOBALVARIABLEWRAPPERS_H
