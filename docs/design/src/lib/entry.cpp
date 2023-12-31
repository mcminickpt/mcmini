#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../demos/type-id.hpp"

extern "C" __attribute__((constructor)) void my_ctor() {
  // Do something here
  void *buf = malloc(10 * sizeof(char));
  memset(buf, 0x31, 10 * sizeof(char));

  ((char *)buf)[9] = 0;
  write(1, buf, 10);
}

extern "C" VISIBLE bool my_func(type_id_t id) {
  return id == type_id<shared_k>();
}

extern "C" VISIBLE bool my_shared_k(std::type_info &info) {
  return info == typeid(shared_k);
}