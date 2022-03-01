#ifndef DPOR_PTHREAD_WRAPPERS_H
#define DPOR_PTHREAD_WRAPPERS_H

#include <pthread.h>
#include "dpor.h"

typeof(&pthread_create) pthread_create_ptr;
#define __real_pthread_create (*pthread_create_ptr)
typeof(&pthread_exit) pthread_exit_ptr;
#define __real_pthread_exit (*pthread_exit_ptr)
typeof(&pthread_join) pthread_join_ptr;
#define __real_pthread_join (*pthread_join_ptr)
typeof(&pthread_mutex_init) pthread_mutex_init_ptr;
#define __real_pthread_mutex_init (*pthread_mutex_init_ptr)
typeof(&pthread_mutex_lock) pthread_mutex_lock_ptr;
#define __real_pthread_mutex_lock (*pthread_mutex_lock_ptr)
typeof(&pthread_mutex_unlock) pthread_mutex_unlock_ptr;
#define __real_pthread_mutex_unlock (*pthread_mutex_unlock_ptr)
typeof(&sem_init) sem_init_ptr;
#define __real_sem_init (*sem_init_ptr)
typeof(&sem_wait) sem_wait_ptr;
#define __real_sem_wait (*sem_wait_ptr)
typeof(&sem_post) sem_post_ptr;
#define __real_sem_post (*sem_post_ptr)

#define MAX_THREAD_INDEX 100

typedef struct dpor_thread dpor_thread_t;
dpor_thread_t dpor_thread[MAX_THREAD_INDEX];
dpor_thread_t *new_thread_blocked(pthread_t thread_id);

int dpor_pthread_mutex_init(pthread_mutex_t*, pthread_mutexattr_t*);
int dpor_pthread_mutex_lock(pthread_mutex_t*);
int dpor_pthread_mutex_unlock(pthread_mutex_t*);
int dpor_pthread_mutex_destroy(pthread_mutex_t*);

int dpor_pthread_create(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *);
int dpor_pthread_join(pthread_t *, void**);

#endif //DPOR_PTHREAD_WRAPPERS_H
