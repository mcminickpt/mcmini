#pragma once

#include <signal.h>

void mc_template_receive_sigchld(int sig, siginfo_t *, void *);
