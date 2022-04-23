#ifndef GMAL_GMALSHAREDLIBRARYWRAPPERS_H
#define GMAL_GMALSHAREDLIBRARYWRAPPERS_H

#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>

extern typeof(&pthread_create) pthread_create_ptr;
extern typeof(&pthread_join) pthread_join_ptr;
extern typeof(&pthread_mutex_init) pthread_mutex_init_ptr;
extern typeof(&pthread_mutex_lock) pthread_mutex_lock_ptr;
extern typeof(&pthread_mutex_unlock) pthread_mutex_unlock_ptr;
extern typeof(&sem_wait) sem_wait_ptr;
extern typeof(&sem_post) sem_post_ptr;
extern typeof(&sem_init) sem_init_ptr;

#define __real_pthread_create (*pthread_create_ptr)
#define __real_pthread_join (*pthread_join_ptr)
#define __real_pthread_mutex_init (*pthread_mutex_init_ptr)
#define __real_pthread_mutex_lock (*pthread_mutex_lock_ptr)
#define __real_pthread_mutex_unlock (*pthread_mutex_unlock_ptr)
#define __real_sem_wait (*sem_wait_ptr)
#define __real_sem_post (*sem_post_ptr)
#define __real_sem_init (*sem_init_ptr)

void gmal_load_pthread_routines();

#endif //GMAL_GMALSHAREDLIBRARYWRAPPERS_H
