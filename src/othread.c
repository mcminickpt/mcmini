//
// Created by parallels on 2/13/22.
//

#include "othread.h"

othread_ref
othread_pthread_wrap(pthread_t pthread)
{
    thread_ref thread = thread_wrap(pthread);
    return othread_thread_wrap(thread);
}

othread_ref
othread_thread_wrap(thread_ref thread)
{


}