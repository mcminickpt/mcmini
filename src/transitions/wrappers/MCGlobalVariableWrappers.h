#ifndef MC_MCGLOBALVARIABLEWRAPPERS_H
#define MC_MCGLOBALVARIABLEWRAPPERS_H

#include "MCShared.h"

MC_EXTERN void *mcmini_read(void *);
MC_EXTERN void mcmini_write(void *, void *);

#define READ(x)             mcmini_read(x)
#define WRITE(old, new)     mcmini_write(old, new)

#endif //MC_MCGLOBALVARIABLEWRAPPERS_H
