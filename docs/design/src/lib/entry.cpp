#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

__attribute__((constructor)) void my_ctor() {
  // Do something here
  void *buf = malloc(10 * sizeof(char));
  memset(buf, 0x31, 10 * sizeof(char));

  ((char *)buf)[9] = 0;
  write(1, buf, 10);
}