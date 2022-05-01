#include "GMALConditionVariableWrappers.h"

extern "C" {
    #include "transitions/wrappers/GMALSharedLibraryWrappers.h"
}

int
gmal_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{

    return __real_pthread_cond_init(cond, attr);
}

int
gmal_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    __real_pthread_mutex_unlock(mutex);
}

int
gmal_pthread_cond_signal(pthread_cond_t*)
{

}

int
gmal_pthread_cond_broadcast(pthread_cond_t*)
{

}