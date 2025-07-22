#pragma once

#include <signal.h>
#include <stdbool.h>

bool is_bad_signal(int signo);
void mc_template_receive_sigchld(int sig, siginfo_t *, void *);
