#ifndef DPOR_MUTEX_H
#define DPOR_MUTEX_H

#include <pthread.h>

enum mutex_state { MUTEX_LOCKED, MUTEX_UNLOCKED, MUTEX_UNKNOWN };
struct mutex {
    enum mutex_state state;
    pthread_mutex_t mutex;
};

#endif //DPOR_MUTEX_H
