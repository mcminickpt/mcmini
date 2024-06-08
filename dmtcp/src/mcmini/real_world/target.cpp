#include "mcmini/real_world/target.hpp"

#include <libgen.h>
#include <sys/personality.h>
#include <sys/prctl.h>

#include <csignal>

using namespace real_world;

void target::execvp() const {
  // NOTE: According to the man page `dirname(const char *path)` "may modify the
  // contents of `path`...", so we use the storage of the local instead.
  std::vector<char> target_program(this->target_program.begin(),
                                   this->target_program.end());
  char buf[1000];
  buf[sizeof(buf) - 1] = '\0';
  snprintf(buf, sizeof buf, "%s:%s/libmcmini.so",
           (getenv("LD_PRELOAD") ? getenv("LD_PRELOAD") : ""),
           dirname(target_program.data()));
  setenv("LD_PRELOAD", buf, 1);

  // `const_cast<>` is needed to call the C-functions here. A new/delete
  // or malloc/free _could be_ needed, we'd need to check the man page. As
  // long as the char * is not actually modified, this is OK and the best way
  // to interface with the C library routines
  std::vector<char*> args;
  args.reserve(this->target_program_args.size());
  args.push_back(const_cast<char*>(this->target_program.c_str()));
  for (const std::string& arg : this->target_program_args)
    args.push_back(const_cast<char*>(arg.c_str()));
  args.push_back(NULL);

  // Ensures that addresses in the template process remain "stable"
  personality(ADDR_NO_RANDOMIZE);

  // Ensures that the template process is sent a SIGTERM if THIS THREAD ever
  // exits. Since McMini is currently single-threaded, this is equivalent to
  // saying if McMini ever exits. Note that this `prctl(2)` persists across
  // `execvp(2)`.
  prctl(PR_SET_PDEATHSIG, SIGTERM);

  // Ensures that the child will accept the reception of all signals (see
  // `install_process_wide_signal_handlers()` where we explicitly block the
  // reception of signals from the main thread i.e. this thread which is
  // calling `fork()`)
  //
  // The man page for `sigprocmask(2)` reads:
  //
  // "A child created via fork(2) inherits a copy of its parent's signal mask;
  // the signal mask is preserved across execve(2)."
  //
  // hence why we clear it about before `execvp()`
  sigset_t empty_set;
  sigemptyset(&empty_set);
  sigprocmask(SIG_SETMASK, &empty_set, NULL);
  ::execvp(this->target_program.c_str(), args.data());
}
