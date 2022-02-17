#include "common.h"
#include <stdio.h>

int
putchars(int c, unsigned int times)
{
    for (unsigned int i = 0; i < times; i++) {
        if (putchar(c) == EOF) return EOF;
    }
    return (unsigned char)c;
}

int
space(unsigned int times)
{
    return putchars(' ', times);
}