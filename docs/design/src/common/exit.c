#include "mcmini/common/exit.h"

#include <pthread.h>
#include <stdlib.h>

void exit_mt(int status) {
    static pthread_mutex_t exit_mut = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&exit_mut);
    exit(status);
    pthread_mutex_unlock(&exit_mut);
}