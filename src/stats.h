#ifndef DPOR_STATS_H
#define DPOR_STATS_H

#include <stdint.h>
#include "decl.h"

STRUCT_DECL(stats);
struct stats {
    uint64_t num_deadlocks;
};

#endif //DPOR_STATS_H
