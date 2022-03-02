#ifndef DPOR_COMMON_H
#define DPOR_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#define FORK_IS_CHILD_PID(pid) (pid == 0)
#define FORK_IS_PARENT_PID(pid) (!(FORK_IS_CHILD_PID(pid)))
#define PTHREAD_SUCCESS (0)
#define SEM_FLAG_SHARED (1)

typedef void*(*thread_routine)(void*);
typedef void(*free_function)(void*);

int putchars(int, unsigned int times);
int space(unsigned int times);


#endif //DPOR_COMMON_H
