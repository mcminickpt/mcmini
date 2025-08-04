#include "mcmini/lib/sig.h"

bool is_bad_signal(int signo) {
    switch (signo) {
    case SIGILL:
    case SIGABRT:
    case SIGBUS:
    case SIGFPE:
    case SIGKILL:
    case SIGSEGV:
    case SIGPIPE:
    case SIGSYS:
      return true;
    default:
      return false;
  }
}
