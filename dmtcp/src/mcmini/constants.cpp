#include "mcmini/constants.hpp"

#include <csignal>
#include <mutex>

pid_t constants::getpid() {
  static std::once_flag once_flag;
  static pid_t pid = -1;
  std::call_once(once_flag, []() { pid = ::getpid(); });
  return pid;
}
