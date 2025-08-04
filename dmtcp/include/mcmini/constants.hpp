#pragma once

#include <sys/types.h>

class constants {
 public:
  static pid_t getpid();
  static size_t subsystem_log_size();
  static size_t file_log_size();
};
