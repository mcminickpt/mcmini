#include <stdio.h>

#include "mcmini/checker/algorithm.hpp"

__attribute__((constructor)) void my_ctor() {
  printf("This is a test\n");

  mcmini::verification::modelchecking::algorithm *algorithm;
}