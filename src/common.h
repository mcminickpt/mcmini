#ifndef DPOR_COMMON_H
#define DPOR_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_TOTAL_THREADS_PER_SCHEDULE 10u
#define MAX_VISIBLE_OPERATION_DEPTH 7u
#define MAX_MUTEX_OBJECT_COUNT 100u

typedef uint64_t tid_t;
typedef uint64_t mutid_t;
#define TID_MAIN_THREAD (0ul)
#define TID_INVALID (-1ul) // ULONG_MAX

#define FORK_IS_CHILD_PID(pid) (pid == 0)
#define FORK_IS_PARENT_PID(pid) (!(FORK_IS_CHILD_PID(pid)))
#define PTHREAD_SUCCESS (0)
#define SEM_FLAG_SHARED (1)

typedef void*(*thread_routine)(void*);
typedef void(*free_function)(void*);

int putchars(int, unsigned int times);
int space(unsigned int times);


#endif //DPOR_COMMON_H
