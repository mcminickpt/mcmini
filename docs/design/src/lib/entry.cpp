#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern "C" __attribute__((constructor)) void my_ctor() {
  // Do something here with the constructor (e.g. dlsym preparation)
  void *buf = malloc(10 * sizeof(char));
  memset(buf, (int)('A'), 10 * sizeof(char));

  ((char *)buf)[9] = 0;
  write(STDERR_FILENO, buf, 10);
}
