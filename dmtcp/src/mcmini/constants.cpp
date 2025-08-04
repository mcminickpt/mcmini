#include "mcmini/constants.hpp"

#include <csignal>
#include <mutex>

pid_t constants::getpid() {
  static std::once_flag once_flag;
  static pid_t pid = -1;
  std::call_once(once_flag, []() { pid = ::getpid(); });
  return pid;
}

size_t constants::subsystem_log_size() {
  // static std::once_flag once_flag;
  // static uint32_t sz = -1;
  // std::call_once(once_flag,
  //                []() { sz = std::getenv("MCMINI_SUBSYS_LOG_CHARS") ? ; });
  // return pid;
  return 5;
}
size_t constants::file_log_size() { return 20; }
