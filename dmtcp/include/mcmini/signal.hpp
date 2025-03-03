#pragma once

#include <atomic>
#include <csignal>
#include <exception>
#include <iostream>
#include <unordered_map>

using signo_t = int;
extern const std::unordered_map<signo_t, const char *> sig_to_str;

struct signal_tracker {
 private:
  static signal_tracker _instance;
  constexpr signal_tracker() = default;
  constexpr static size_t MAX_SIGNAL_TYPES = NSIG - 1;
#ifndef ATOMIC_INT_LOCK_FREE
#error "Signal tracking relies on `std::atomic` to be lock-free."
#endif
  std::atomic_uint32_t flags[MAX_SIGNAL_TYPES] = {};

 public:
  struct interrupted_error;
  static signal_tracker &instance();
  static void throw_if_received(int sig);
  void set_signal(int sig);
  bool has_signal(int sig) const;
  bool try_consume_signal(int sig);
};

extern "C" void signal_tracker_sig_handler(int sig);
extern "C" void install_process_wide_signal_handlers();
