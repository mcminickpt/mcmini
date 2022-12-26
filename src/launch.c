#include "mcmini/MCEnv.h"
#include <assert.h>
#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
  char **cur_arg = &argv[1];
  if (argc == 1) {
    cur_arg[0] = "--help";
    cur_arg[1] = NULL;
  }

  // TODO: Use argp.h instead (more options, better descriptions, etc)
  while (cur_arg[0] != NULL && cur_arg[0][0] == '-') {
    if (strcmp(cur_arg[0], "--max-depth-per-thread") == 0 ||
        strcmp(cur_arg[0], "-m") == 0) {
      // FIXME: "env_max_thread_depth":
      //           Just use "MCMINI_MAX_DEPTH_PER_THREAD" below, everywhere.
      //           There's no need to add a '#define' to hide the env var name.
      setenv(ENV_MAX_THREAD_DEPTH, cur_arg[1], 1);
      cur_arg += 2;
    }
    else if (cur_arg[0][1] == 'm' && isdigit(cur_arg[0][2])) {
      setenv(ENV_MAX_THREAD_DEPTH, cur_arg[0] + 2, 1);
      cur_arg++;
    }
    else if (strcmp(cur_arg[0], "--debug-at-trace") == 0 ||
             strcmp(cur_arg[0], "-d") == 0) {
      // FIXME: "env_debug_at_trace": Just use "MCMINI_DEBUG_AT_TRACE" below.
      //        There's no need to add a '#define' to hide the env var name.
      setenv(ENV_DEBUG_AT_TRACE, cur_arg[1], 1);
      cur_arg += 2;
    }
    else if (cur_arg[0][1] == 'd' && isdigit(cur_arg[0][2])) {
      setenv(ENV_DEBUG_AT_TRACE, cur_arg[0] + 2, 1);
      cur_arg++;
    }
    else if (strcmp(cur_arg[0], "--verbose") == 0 ||
             strcmp(cur_arg[0], "-v") == 0) {
      // FIXME: ENV_VERBOSE: Just use "MCMINI_VERBOSE" below and everywhere.
      //        There's no need to add a '#define' to hide the env var name.
      setenv(ENV_VERBOSE, "1", 1);
      cur_arg++;
    }
    else if (strcmp(cur_arg[0], "--first-deadlock") == 0 ||
             strcmp(cur_arg[0], "--first") == 0 ||
             strcmp(cur_arg[0], "-f") == 0) {
      // FIXME: ENV_STOP_AT_FIRST_DEADLOCK: Just use "MCMINI_FIRST_DEADLOCK"
      //           below and everywhere.
      //        There's no need to add a '#define' to hide the env var name.
      setenv(ENV_STOP_AT_FIRST_DEADLOCK, "1", 1);
      cur_arg++;
    }
    else if (strcmp(cur_arg[0], "--check-forward-progress") == 0 ||
             strcmp(cur_arg[0], "-c") == 0) {
      setenv(ENV_CHECK_FORWARD_PROGRESS, "1", 1);
      cur_arg++;
    }
    else if (strcmp(cur_arg[0], "--long-test") == 0) {
      setenv(ENV_LONG_TEST, "1", 1);
      cur_arg++;
    }
    else if (strcmp(cur_arg[0], "--print-at-trace") == 0) {
      setenv(ENV_PRINT_AT_TRACE, cur_arg[1], 1);
      cur_arg += 2;
    }
    else if (strcmp(cur_arg[0], "--help") == 0 ||
             strcmp(cur_arg[0], "-h") == 0) {
      fprintf(stderr,
              "Usage: mcmini [--max-depth-per-thread|-m <num>] "
              "[--debug-at-trace|-d <num>]\n"
              "[--first-deadlock|-first] [--print-at-trace]\n"
              "[--verbose|-v] [--help|-h] target_executable\n");
      exit(1);
    }
    else {
      printf("mcmini: unrecognized option: %s\n", cur_arg[0]);
      exit(1);
    }
  }

  struct stat stat_buf;
  if (cur_arg[0] == NULL || stat(cur_arg[0], &stat_buf) == -1) {
    fprintf(stderr,
            "*** Missing target_executable or no such file.\n\n");
    exit(1);
  }

  char buf[1000];
  buf[sizeof(buf) - 1] = '\0';
  // We add ours to the end of any PRELOAD of the target application.
  snprintf(buf, sizeof buf, "%s:%s/libmcmini.so",
           (getenv("LD_PRELOAD") ? getenv("LD_PRELOAD") : ""),
           dirname(argv[0]));
  // Guard against buffer overrun:
  assert(buf[sizeof(buf) - 1] == '\0');
  setenv("LD_PRELOAD", buf, 1);
  printf("About to exec into %s\n", cur_arg[0]);
  fflush(stdout);
  execvp(cur_arg[0], cur_arg);
  fprintf(stderr, "Executable '%s' not found.\n", cur_arg[0]);
  perror("mcmini");
  return 1;
}
