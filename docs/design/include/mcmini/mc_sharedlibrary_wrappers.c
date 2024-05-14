#define _GNU_SOURCE
#include "docs/design/include/mcmini/mc_sharedlibrary_wrappers.h"
#include <errno.h>
#include <stdio.h>
#include <stdatomic.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

typeof(&pthread_create) pthread_create_ptr;
typeof(&pthread_join) pthread_join_ptr;
typeof(&pthread_mutex_init) pthread_mutex_init_ptr;
typeof(&pthread_mutex_lock) pthread_mutex_lock_ptr;
typeof(&pthread_mutex_unlock) pthread_mutex_unlock_ptr;
typeof(&sem_wait) sem_wait_ptr;
typeof(&sem_post) sem_post_ptr;
typeof(&sem_init) sem_init_ptr;
typeof(&sem_destroy) sem_destroy_ptr;
__attribute__((__noreturn__)) typeof(&exit) exit_ptr;
__attribute__((__noreturn__)) typeof(&abort) abort_ptr;
//extern typeof(&pthread_barrier_init) pthread_barrier_init_ptr;
//extern typeof(&pthread_barrier_wait) pthread_barrier_wait_ptr;
typeof(&pthread_cond_init) pthread_cond_init_ptr;
typeof(&pthread_cond_wait) pthread_cond_wait_ptr;
typeof(&pthread_cond_signal) pthread_cond_signal_ptr;
typeof(&pthread_cond_broadcast) pthread_cond_broadcast_ptr;
//extern typeof(&pthread_rwlock_init) pthread_rwlock_init_ptr;
//extern typeof(&pthread_rwlock_rdlock) pthread_rwlock_rdlock_ptr;
//extern typeof(&pthread_rwlock_wrlock) pthread_rwlock_wrlock_ptr;
//extern typeof(&pthread_rwlock_unlock) pthread_rwlock_unlock_ptr;
typeof(&sleep) sleep_ptr;

int mcmini_enabled = -1;
int fd;
const char *shared_mem_obj = "/sh_mem_obj";
int count_calls = 0;
void *base_address = (void *)0x10004567;
int flag = -1;

// Function to check the environment variable and initialize mcmini_enabled
void check_mcmini_mode() {
    if (mcmini_enabled == -1) {
        const char* env_enabled = getenv("MCMINI_RECORD");
        if (env_enabled && strcmp(env_enabled, "1") == 0) mcmini_enabled=0;
    }

    // file descriptor for the shared memory used for mmap
    fd = shm_open(shared_mem_obj, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    	if (fd == -1) {
        	perror("shm_open");
        	exit(EXIT_FAILURE);
    	}

    // Set the size of the shared memory object to the size of the mutex
    if (ftruncate(fd, sizeof(pthread_mutex_t)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
}

// List to store the addresses where mutex is mapped during recording

struct Rec_list {
    pthread_mutex_t *mutex;
    int state; // 0 - unlocked, 1 - locked
    struct Rec_list *next;
};

typedef struct Rec_list Rec_list;
Rec_list *head = NULL;
Rec_list *current = NULL;


void
mc_load_intercepted_symbol_addresses()
{  
  pthread_create_ptr       = dlsym(RTLD_NEXT, "pthread_create");
  pthread_join_ptr         = dlsym(RTLD_NEXT, "pthread_join");
  pthread_mutex_init_ptr   = dlsym(RTLD_NEXT, "pthread_mutex_init");
  pthread_mutex_lock_ptr   = dlsym(RTLD_NEXT, "pthread_mutex_lock");
  pthread_mutex_unlock_ptr = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
  sem_wait_ptr             = dlsym(RTLD_NEXT, "sem_wait");
  sem_post_ptr             = dlsym(RTLD_NEXT, "sem_post");
  sem_init_ptr             = dlsym(RTLD_NEXT, "sem_init");
  exit_ptr                 = dlsym(RTLD_NEXT, "exit");
  abort_ptr                = dlsym(RTLD_NEXT, "abort");
//   pthread_barrier_init_ptr = dlsym(RTLD_NEXT, "pthread_barrier_init");
//   pthread_barrier_wait_ptr = dlsym(RTLD_NEXT, "pthread_barrier_wait");
//   pthread_rwlock_init_ptr  = dlsym(RTLD_NEXT, "pthread_rwlock_init");
//   pthread_rwlock_rdlock_ptr =
//     dlsym(RTLD_NEXT, "pthread_rwlock_rdlock");
//   pthread_rwlock_wrlock_ptr =
//     dlsym(RTLD_NEXT, "pthread_rwlock_wrlock");
//   pthread_rwlock_unlock_ptr =
//     dlsym(RTLD_NEXT, "pthread_rwlock_unlock");
  pthread_cond_init_ptr   = dlsym(RTLD_NEXT, "pthread_cond_init");
  pthread_cond_wait_ptr   = dlsym(RTLD_NEXT, "pthread_cond_wait");
  pthread_cond_signal_ptr = dlsym(RTLD_NEXT, "pthread_cond_signal");
  pthread_cond_broadcast_ptr =
    dlsym(RTLD_NEXT, "pthread_cond_broadcast");
  sleep_ptr = dlsym(RTLD_NEXT, "sleep");
}

int
pthread_create(pthread_t *pthread, const pthread_attr_t *attr,
               void *(*routine)(void *), void *arg)
{
  if(mcmini_enabled) return mc_pthread_create(pthread, attr, routine, arg);
  else {
	  /*pthread_create_ptr = dlsym(RTLD_NEXT, "pthread_create");
	  return pthread_create_ptr;*/
	  int return_value =  __real_pthread_create(pthread, attr, routine, arg);
	  return return_value;
	}
}

int
pthread_join(pthread_t pthread, void **result)
{
  if(mcmini_enabled) return mc_pthread_join(pthread, result);
  else return __real_pthread_join(pthread, result);
}

int
pthread_mutex_init(pthread_mutex_t *mutex,
                   const pthread_mutexattr_t *mutexattr)
{  if(mcmini_enabled) return mc_pthread_mutex_init(mutex, mutexattr);
  else {
	  //pthread_mutex_init_ptr   = dlsym(RTLD_NEXT, "pthread_mutex_init");
	  //return pthread_mutex_init_ptr;
	  int result =  __real_pthread_mutex_init(mutex, mutexattr);
    return result;
  }
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
  if(mcmini_enabled) return mc_pthread_mutex_lock(mutex);
  else {  
	  int return_value = __real_pthread_mutex_lock(mutex);
	  count_calls++;
	  /*
	   * Map the shared memory into the memory with MAP_FIXED_NOREPLACE
	   * Recording phase.
	   */
	  long pagesize = sysconf(_SC_PAGESIZE);
	  void *desired_address = (void *)((uintptr_t)base_address + 64*count_calls * sizeof(pthread_mutex_t));
	  //printf("\n -- D.A after count calls %p ---", desired_address);
	  desired_address = (void *)(((uintptr_t)desired_address / pagesize) * pagesize);
	  //printf("\n -- D.A after page adjustment %p ---", desired_address);
	  int retry_count = 0;
	  size_t offset = sizeof(pthread_mutex_t);
    while (1) {
        // Map the shared memory into memory at the desired address
        mutex = (pthread_mutex_t *)mmap(desired_address, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED_NOREPLACE, fd, 0);
        if (mutex == MAP_FAILED) {
            if (errno == EEXIST) {
		    retry_count++;
                // Address already in use, try again with a different address
                desired_address = (void *)((uintptr_t)desired_address + (retry_count*64)*offset);
		//printf("\n -- D.A after retry count %p --", desired_address);
		desired_address = (void *)(((uintptr_t)desired_address / pagesize) * pagesize);
		//printf("\n ---- new desired address for mmap is %p ------", desired_address); 

      
                if (retry_count >= 10) {
                    printf("Maximum retries reached. Unable to find a free address.\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                perror("mmap");
                exit(EXIT_FAILURE);
            }
        } else {
            // Mapping successful, break the retry loop
	    printf("\n Mapped mutex address : %p", mutex);
            break;
        }
    }
       
	// append the address to the linked list
        Rec_list *newNode = (Rec_list *)malloc(sizeof(Rec_list));
	if (newNode == NULL) {
    	  perror("malloc");
    	  exit(EXIT_FAILURE);
	}
  //recording the address of mutex and its state
	newNode->mutex = mutex;
	newNode->next = NULL; // Ensure the new node's next pointer is initialized to NULL
  if (return_value == 0) {
    newNode->state = 1;
  } 
	if (head == NULL) {
    	  head = newNode;
    	  current = head;
	} else {
    	    current->next = newNode; // Update the next pointer of the current node
    	    current = newNode; // Move the current pointer to the newly added node
	}

	//print the list
	//printLinkedList(head);
	printf("\n Mapped mutex address successfully saved in the linked list: %p\n", newNode->mutex);	
       return return_value;
  }
}

int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  if(mcmini_enabled) return mc_pthread_mutex_unlock(mutex);
  else return __real_pthread_mutex_unlock(mutex);
}

int
sem_init(sem_t *sem, int pshared, unsigned int value)
{
  if(mcmini_enabled) return mc_sem_init(sem, pshared, value);
  else return __real_sem_init(sem, pshared, value);
}

int
sem_post(sem_t *sem)
{
  if(mcmini_enabled) return mc_sem_post(sem);
  else return __real_sem_post(sem);
}

int
sem_wait(sem_t *sem)
{
  if(mcmini_enabled) return mc_sem_wait(sem);
  else return __real_sem_wait(sem);
}

void
exit(int status)
{
  if(mcmini_enabled) mc_transparent_exit(status);
  else __real_exit(status);
}

void
abort()
{
  if(mcmini_enabled) mc_transparent_abort();
  else __real_abort();
}

// int
// pthread_barrier_init(pthread_barrier_t *barrier,
//                      const pthread_barrierattr_t *attr,
//                      unsigned int count)
// {
//   if(mcmini_enabled) return mc_pthread_barrier_init(barrier, attr, count);
//   else return __real_pthread_barrier_init(barrier, attr, count);
// }

// int
// pthread_barrier_wait(pthread_barrier_t *barrier)
// {
//   if(mcmini_enabled) return mc_pthread_barrier_wait(barrier);
//   else return __real_pthread_barrier_wait(barrier);
// }

// int
// pthread_rwlock_init(pthread_rwlock_t *rwlock,
//                     const pthread_rwlockattr_t *attr)
// {
//   if(mcmini_enabled) return mc_pthread_rwlock_init(rwlock, attr);
//   else return __real_pthread_rwlock_init(rwlock, attr);
// }

// int
// pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
// {
//   if(mcmini_enabled) return mc_pthread_rwlock_rdlock(rwlock);
//   else return __real_pthread_rwlock_rdlock(rwlock);
// }

// int
// pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
// {
//   if(mcmini_enabled) return mc_pthread_rwlock_wrlock(rwlock);
//   else   return __real_pthread_rwlock_wrlock(rwlock);
//   }

// int
// pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
// {
//   if(mcmini_enabled) return mc_pthread_rwlock_unlock(rwlock);
//   else  return __real_pthread_rwlock_unlock(rwlock);
// }

// int
// pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
// {
//   if(!mcmini_enabled) return mc_pthread_rwlock_destroy(rwlock);
//   else typeof(&pthread_rwlock_destroy) pthread_rwlock_destroy_ptr = dlsym(RTLD_NEXT, "pthread_rwlock_destroy");
// }

int
pthread_cond_init(pthread_cond_t *cond,
                  const pthread_condattr_t *attr)
{
  if(mcmini_enabled) return mc_pthread_cond_init(cond, attr);
  else return __real_pthread_cond_init(cond, attr);
}

int
pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  if(mcmini_enabled) return mc_pthread_cond_wait(cond, mutex);
  else return __real_pthread_cond_wait(cond, mutex);
}

int
pthread_cond_signal(pthread_cond_t *cond)
{
  if(mcmini_enabled) return mc_pthread_cond_signal(cond);
  else return __real_pthread_cond_signal(cond);
}

int
pthread_cond_broadcast(pthread_cond_t *cond)
{
  if(mcmini_enabled) return mc_pthread_cond_broadcast(cond);
  else  return __real_pthread_cond_broadcast(cond);
}

unsigned int
sleep(unsigned int seconds)
{
  /* Treat it as if no  time passed */
  return 0;
}

// int
// pthread_rwwlock_init(pthread_rwwlock_t *rwwlock)
// {
//   if(mcmini_enabled) return mc_pthread_rwwlock_init(rwwlock);

// }

// int
// pthread_rwwlock_rdlock(pthread_rwwlock_t *rwwlock)
// {
//   if(mcmini_enabled) return mc_pthread_rwwlock_rdlock(rwwlock);
// }

// int
// pthread_rwwlock_wr1lock(pthread_rwwlock_t *rwwlock)
// {
//   if(mcmini_enabled) return mc_pthread_rwwlock_wr1lock(rwwlock);
// }

// int
// pthread_rwwlock_wr2lock(pthread_rwwlock_t *rwwlock)
// {
//   if(mcmini_enabled) return mc_pthread_rwwlock_wr2lock(rwwlock);
// }

// int
// pthread_rwwlock_unlock(pthread_rwwlock_t *rwwlock)
// {
//   if(mcmini_enabled) return mc_pthread_rwwlock_unlock(rwwlock);
// }

// int
// pthread_rwwlock_destroy(pthread_rwwlock_t *rwwlock)
// {
//   if(mcmini_enabled) return mc_pthread_rwwlock_destroy(rwwlock);
// }


