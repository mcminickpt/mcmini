#include <stdio.h>

__attribute__((constructor)) void my_ctor() { printf("This is a test\n"); }