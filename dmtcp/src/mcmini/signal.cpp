#include "mcmini/signal.hpp"

#include <bits/sigaction.h>
#include <bits/types/siginfo_t.h>
#include <signal.h>

#include <atomic>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

const std::unordered_map<signo_t, const char *> sig_to_str = {
    {SIGINT, "SIGINT"},
    {SIGCHLD, "SIGCHLD"},
    {SIGKILL, "SIGKILL"},
    {SIGUSR1, "SIGUSR1"},
    {SIGUSR2, "SIGUSR2"}};

void signal_tracker_sig_handler(int sig, siginfo_t *, void *) {
  signal_tracker::instance().set_signal(sig);
}

// static void handle_incoming_signals(const sigset_t blocking_set) {
//   sigset_t accept_all;
//   sigemptyset(&accept_all);
//   pthread_sigmask(SIG_SETMASK, &accept_all, NULL);

//   struct sigaction action;
//   action.sa_flags = SA_SIGINFO;
//   action.sa_sigaction = &signal_tracker_sig_handler;
//   sigemptyset(&action.sa_mask);
//   sigaction(SIGCHLD, &action, NULL);
//   sigaction(SIGINT, &action, NULL);
//   sigaction(SIGUSR1, &action, NULL);
//   sigaction(SIGUSR2, &action, NULL);

//   while (true) {
//     int sig;
//     sigwait(&blocking_set, &sig);

//     if (sig == SIGINT) {
//       std::exit(EXIT_FAILURE);
//     }
//   }
// }

void install_process_wide_signal_handlers() {
  // From
  // "https://wiki.sei.cmu.edu/confluence/display/cplusplus/MSC54-CPP.+A+signal+handler+must+be+a+plain+old+function"
  //
  // """
  // The common subset of the C and C++ languages consists of all
  // declarations, definitions, and expressions that may appear in a
  // well-formed C++ program and also in a conforming C program. A POF
  // (“plain old function”) is a function that uses only features from this
  // common subset, and that does not directly or indirectly use any
  // function that is not a POF, except that it may use plain lock-free
  // atomic operations. A plain lock-free atomic operation is an invocation
  // of a function f from Clause 29, such that f is not a member function,
  // and either f is the function atomic_is_lock_free, or for every atomic
  // argument A passed to f, atomic_is_lock_free(A) yields true. All signal
  // handlers shall have C linkage. The behavior of any function other than
  // a POF used as a signal handler in a C++ program is
  // implementation-defined.
  // """

  // The main thread blocks the reception of all signals and instead delegates
  // this to a background thread.
  //
  // NOTE: This thread will not be present when `fork()`-ing this process (see
  // the man page) which is the intended behavior.
  sigset_t all_signals;
  sigfillset(&all_signals);
  // pthread_sigmask(SIG_SETMASK, &all_signals, NULL);
  // std::thread(&handle_incoming_signals, all_signals).detach();
}

signal_tracker &signal_tracker::instance() {
  static signal_tracker instance;
  return instance;
}

void signal_tracker::set_signal(int sig) {
  flags[sig].fetch_add(1, std::memory_order_relaxed);
}

bool signal_tracker::has_signal(int sig) const {
  return flags[sig].load(std::memory_order_relaxed) > 0;
}

bool signal_tracker::try_consume_signal(int sig) {
  uint32_t current = flags[sig].load(std::memory_order_relaxed);
  while (current > 0) {
    if (flags[sig].compare_exchange_weak(current, current - 1,
                                         std::memory_order_release,
                                         std::memory_order_relaxed)) {
      return true;
    }
  }
  return false;  // No signal to consume
}

//*** Interrupted Error ***//

struct signal_tracker::interrupted_error : public std::exception {
 private:
  std::string msg;

 public:
  explicit interrupted_error(signo_t sig) {
    msg = "Interrupted with signal " + std::to_string(sig);
    if (sig_to_str.count(sig) != 0) {
      msg += " (";
      msg += sig_to_str.find(sig)->second;
      msg += ")";
    } else {
      msg += " (unknown signal name)";
    }
  }
  const char *what() const noexcept override { return msg.c_str(); }
};

void signal_tracker::throw_if_received(int sig) {
  if (instance().try_consume_signal(sig)) {
    throw interrupted_error(sig);
  }
}
