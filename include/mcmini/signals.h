#ifndef INCLUDE_MCMINI_SIGNALS_HPP
#define INCLUDE_MCMINI_SIGNALS_HPP

#include <signal.h>

/**
 * @brief Registers signal handlers for a trace process
 *
 * In addition to registering signal handlers for a trace, all signal
 * handlers previously registered for the scheduler are set back to
 * their default value. This prevents signal handlers intended only to
 * be registered with the scheduler process from getting called in the
 * trace
 *
 * @return 0 if all signal handlers were successfully installed;
 * otherwise a nonzero value is returned
 */
int install_sighandles_for_trace();
void sigusr1_handler_trace(int sig);
void sigusr2_handler_trace(int sig);

/**
 * @brief Registers signal handlers for the scheduler process
 *
 * @return 0 if all signal handlers were successfully installed;
 * otherwise a nonzero value is returned
 */
int install_sighandles_for_scheduler();
void sigint_handler_scheduler(int sig);
void sigusr1_handler_scheduler(int sig);
void sigchld_handler_scheduler(int sig, siginfo_t *, void *);

#endif // INCLUDE_MCMINI_SIGNALS_HPP
