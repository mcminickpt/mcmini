#ifndef DPOR_COMMON_H
#define DPOR_COMMON_H

#include <stdint.h>
#include <stdbool.h>

typedef void*(*thread_routine)(void*);
typedef void(*free_function)(void*);

int putchars(int, unsigned int times);
int space(unsigned int times);

#endif //DPOR_COMMON_H
