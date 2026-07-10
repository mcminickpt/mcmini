#ifndef MC_MCGLOBALVARIABLEWRAPPERS_H
#define MC_MCGLOBALVARIABLEWRAPPERS_H

#include "MCShared.h"

MC_EXTERN void *mcmini_read(void *, char *);
MC_EXTERN void mcmini_write(void *, char *, uint64_t value);

#define MC_TRACK_READ(x)         mcmini_read(&(x), #x)
#define MC_TRACK_WRITE(x, x_new) mcmini_write(&(x), #x, (uint64_t)(x_new))

#endif // MC_MCGLOBALVARIABLEWRAPPERS_H
