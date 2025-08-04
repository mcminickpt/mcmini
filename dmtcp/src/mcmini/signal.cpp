#include "mcmini/signal.hpp"

#include <bits/sigaction.h>
#include <bits/types/siginfo_t.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <atomic>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

const std::unordered_map<signo_t, const char *> sig_to_str = {
    {SIGHUP, "SIGHUP"},       {SIGINT, "SIGINT"},       {SIGQUIT, "SIGQUIT"},
    {SIGILL, "SIGILL"},       {SIGTRAP, "SIGTRAP"},     {SIGABRT, "SIGABRT"},
    {SIGBUS, "SIGBUS"},       {SIGFPE, "SIGFPE"},       {SIGKILL, "SIGKILL"},
    {SIGUSR1, "SIGUSR1"},     {SIGSEGV, "SIGSEGV"},     {SIGUSR2, "SIGUSR2"},
    {SIGPIPE, "SIGPIPE"},     {SIGALRM, "SIGALRM"},     {SIGTERM, "SIGTERM"},
    {SIGSTKFLT, "SIGSTKFLT"}, {SIGCHLD, "SIGCHLD"},     {SIGCONT, "SIGCONT"},
    {SIGSTOP, "SIGSTOP"},     {SIGTSTP, "SIGTSTP"},     {SIGTTIN, "SIGTTIN"},
    {SIGTTOU, "SIGTTOU"},     {SIGURG, "SIGURG"},       {SIGXCPU, "SIGXCPU"},
    {SIGXFSZ, "SIGXFSZ"},     {SIGVTALRM, "SIGVTALRM"}, {SIGPROF, "SIGPROF"},
    {SIGWINCH, "SIGWINCH"},   {SIGIO, "SIGIO"},
#ifdef SIGPOLL
    {SIGPOLL, "SIGPOLL"},
#endif
    {SIGPWR, "SIGPWR"},       {SIGSYS, "SIGSYS"},
};

sem_t *signal_tracker::current_sem = nullptr;

void signal_tracker_sig_handler(int sig, siginfo_t *, void *) {
  // TODO: If multiple SIGCHLDs are received before the main thread has a chance
  // to unset the `current_sem`, the `current_sem` may receive _multiple`
  // `sem_post(2)` calls. We should handle this in the future if we want to
  // retry creating the template process, but for now since we simply fail
  // completely if the template/branch process fails unexpectedly, calling
  // `sem_post(2)` more than once is OK (since we'll just error out anyway).
  signal_tracker::instance().set_signal(sig);
  sem_t *set_sem = signal_tracker::instance().current_sem;
  if (set_sem) {
    // Reset the global value BEFORE posting. We assume only TWO threads inside
    // of McMini at this point.
    sem_post(set_sem);
  }
}

bool signal_tracker::is_bad_signal(int signo) {
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

int signal_tracker::sig_semwait(sem_t *sem) {
  // INVARIANT: Assumes a single thread calls this at once.
  // The McMini process contains a dedicated background thread to receive
  // signals and post to the given semaphore.
  int rc = sem_wait(sem);
  while (rc != 0 && errno == EINTR) {
    rc = sem_wait(sem);
  }
  return rc;
}

static bool is_likely_debugging() {
  // The Linux man page says that `proc/[pid]/status` contains a `TracerPid:
  // field which indicates the process ID tracing this one or `0` if no
  // tracer  is present. We use this as a heuristic to detect is a debugger is
  // present to allow SIGINT to enter the debugger.
  std::ifstream status_file("/proc/self/status");
  if (!status_file.is_open()) {
    return false;  // Cannot open, assume no debugger
  }

  std::string line;
  while (std::getline(status_file, line)) {
    std::istringstream iss(line);
    std::string key;
    iss >> key;
    if (key == "TracerPid:") {
      pid_t tracer_pid = 0;
      iss >> tracer_pid;
      return tracer_pid != 0;
    }
  }
  return false;  // Default: no debugger
}

static void handle_incoming_signals(sem_t *rendez_vous) {
  sigset_t no_signals;
  sigemptyset(&no_signals);
  pthread_sigmask(SIG_SETMASK, &no_signals, NULL);
  pthread_setname_np(pthread_self(), "McMini Signal Listener");
  sem_post(rendez_vous);

  sigset_t all_signals;
  sigfillset(&all_signals);

  // SIGCHLD is handled in a dedicated handler.
  // "Bad" signals (e.g. SIGSEGV) shouldn't be captured by `sigwait()` and
  // should instead lead to default program exiting behaviors
  sigdelset(&all_signals, SIGCHLD);
  sigdelset(&all_signals, SIGTSTP);
  for (const auto sig_pair : sig_to_str) {
    if (signal_tracker::is_bad_signal(sig_pair.first)) {
      sigdelset(&all_signals, sig_pair.first);
    }
  }

  while (true) {
    // According to the `sigwait()` man page:
    // """
    // The  sigwait()  function  suspends execution of the calling thread
    // until one of the signals specified in the signal set set becomes pending.
    // The function accepts the signal (removes it from the pending list of
    // signals),  and  returns the signal number in sig
    // """
    int sig;
    sigwait(&all_signals, &sig);
    std::cout << "Received signal: " << sig_to_str.at(sig) << std::endl;
    if (signal_tracker::is_bad_signal(sig)) {
      std::terminate();
    } else if (sig == SIGINT) {
      if (is_likely_debugging()) {
        std::raise(SIGSTOP);
      } else {
        std::exit(EXIT_FAILURE);
      }
    }
  }
}

void signal_tracker::install_process_wide_signal_handlers() {
  // From
  // "https://wiki.sei.cmu.edu/confluence/display/cplusplus/MSC54-CPP.+A+signal+handler+must+be+a+plain+old+function"
  //
  // """
  // The common subset of the C and C++ languages consists of all
  // declarations, definitions, and expressions that may appear in a
  // well-formed C++ program and also in a conforming C program. A POF
  // (“plain old function”) is a function that uses only features from this
  // common subset, and that does not directly or indirectly use any
  // function that is not a POF, __except that it may use plain lock-free
  // atomic operations__. A plain lock-free atomic operation is an invocation
  // of a function f from Clause 29, such that f is not a member function,
  // and either f is the function atomic_is_lock_free, or for every atomic
  // argument A passed to f, atomic_is_lock_free(A) yields true. All signal
  // handlers shall have C linkage. The behavior of any function other than
  // a POF used as a signal handler in a C++ program is
  // implementation-defined.
  // """
  //
  // Moreover, only async-signal-safe C functions may be called from handlers.
  // Included in this list (according to the `signal-safety(7)` man page) is
  // `sem_post(2)` which is used to unblock the main thread wainting on a child
  // process (e.g. the template process)
  struct sigaction action = {0};
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_sigaction = signal_tracker_sig_handler;

  // See the Linux ma page for `sigaction(2)`:
  //
  // """
  // SA_NOCLDSTOP:
  //
  // If signum is SIGCHLD, do not receive notification when child processes stop
  // (i.e., when they receive one  of SIGSTOP,  SIGTSTP,  SIGTTIN, or SIGTTOU)
  // or resume (i.e., they receive SIGCONT) (see wait(2)).  This flag is
  // meaningful only when establishing a handler for SIGCHLD.
  // """
  action.sa_flags |= SA_NOCLDSTOP;
  sigaction(SIGCHLD, &action, nullptr);

  // IMPORTANT: A SIGCHLD signal will be set to the PENDING status if there are
  // no available threads to receive the signal. One such condition blocking
  // signal delivery is "waiting on a blocking system call", e.g. `sem_wait(2)`.
  // McMini has traditionally been a single-threaded process, but because McMini
  // regularly communicates with child processes that can fail, we need to be
  // able to unblock McMini in case these child processes exit.
  static sem_t rendez_vous;
  sigset_t all_signals;
  sigfillset(&all_signals);
  pthread_sigmask(SIG_SETMASK, &all_signals, nullptr);

  // The `rendez_vous` semaphore ensures that, before continuing execution, the
  // signal thread has been configured to accept all signals.
  sem_init(&rendez_vous, 0, 0);
  std::thread(&handle_incoming_signals, &rendez_vous).detach();
  sem_wait(&rendez_vous);
}

signal_tracker &signal_tracker::instance() {
  // C++ guarantees that `instance` is instantiated in a thread-safe manner
  // prior to `main()`. Inside the McMini process, this behavior is perfectly
  // safe. Dancing around static instance initialization is precisely why
  // `libmcmini.so` is written in C however...
  static signal_tracker instance;
  return instance;
}

void signal_tracker::set_signal(int sig) { flags[sig].fetch_add(1); }

bool signal_tracker::has_signal(int sig) const { return flags[sig].load() > 0; }

bool signal_tracker::try_consume_signal(int sig) {
  // TODO: More relaxed semantics may be possible here, but sequential
  // consistency is the "easiest"
  uint32_t current = flags[sig].load();
  while (current > 0) {
    if (flags[sig].compare_exchange_weak(current, current - 1,
                                         std::memory_order_seq_cst,
                                         std::memory_order_seq_cst)) {
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
