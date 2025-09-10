/**
 * SYNOPSIS: A small program which performs an
 * `exec()` call into the program specified but first
 * ensuring that libmcmini.so is preloadaed
*/

#include <assert.h>
#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "MCEnv.h"
#include "MCConstants.h"
#include "config.h"

int
main(int argc, char *argv[])
{
  int width = strlen(PACKAGE_BUGREPORT) + 4;
  char * stars = "***********************************************"
                 "***********************************************";
  printf(" %.*s\n * %s\n"
         " * Copyright(c) Maxwell Pirtle, Luka Jovanovic, Gene Cooperman\n"
         " *   (LGPLv3 license: See COPYING.md in source code distribution.)\n"
         " * %s\n * %s\n %.*s\n\n", width, stars,
         PACKAGE_STRING, PACKAGE_BUGREPORT, PACKAGE_URL,
         width, stars);

  char **cur_arg = &argv[1];
  if (argc == 1) {
    cur_arg[0] = "--help";
    cur_arg[1] = NULL;
  }

  // Default:  stop on first deadlock
  setenv(ENV_FIRST_DEADLOCK, "1", 1);

  // TODO: Use argp.h instead (more options, better descriptions, etc)
  while (cur_arg[0] != NULL && cur_arg[0][0] == '-') {
    if (strcmp(cur_arg[0], "--max-transitions-depth-limit") == 0 ||
        strcmp(cur_arg[0], "-M") == 0) {
      setenv(ENV_MAX_TRANSITIONS_DEPTH_LIMIT, cur_arg[1], 1);
      char *endptr;
      if (strtol(cur_arg[1], &endptr, 10) == 0 && endptr[0] != '\0') {
        fprintf(stderr, "%s: illegal value\n", "--max-transitions-depth-limit");
        exit(1);
      }
      cur_arg += 2;
    }
    else if (cur_arg[0][1] == 'M' && isdigit(cur_arg[0][2])) {
      setenv(ENV_MAX_TRANSITIONS_DEPTH_LIMIT, cur_arg[0] + 2, 1);
      cur_arg++;
    }
    else if (strcmp(cur_arg[0], "--max-depth-per-thread") == 0 ||
        strcmp(cur_arg[0], "-m") == 0) {
      setenv(ENV_MAX_DEPTH_PER_THREAD, cur_arg[1], 1);
      char *endptr;
      if (strtol(cur_arg[1], &endptr, 10) == 0 && endptr[0] != '\0') {
        fprintf(stderr, "%s: illegal value\n", "--max-depth-per-thread");
        exit(1);
      }
      cur_arg += 2;
    }
    else if (cur_arg[0][1] == 'm' && isdigit(cur_arg[0][2])) {
      setenv(ENV_MAX_DEPTH_PER_THREAD, cur_arg[0] + 2, 1);
      cur_arg++;
    }
    else if (cur_arg[0][1] == 'd' && isdigit(cur_arg[0][2])) {
      setenv(ENV_DEBUG_AT_TRACE_ID, cur_arg[0] + 2, 1);
      cur_arg++;
    }
    else if (strcmp(cur_arg[0], "--verbose") == 0 ||
             strcmp(cur_arg[0], "-v") == 0) {
      if (getenv(ENV_VERBOSE)) {
        setenv(ENV_VERBOSE, "2", 1);
      } else {
        setenv(ENV_VERBOSE, "1", 1);
      }
      cur_arg++;
    }
    else if (strcmp(cur_arg[0], "--all-deadlocks") == 0 ||
             strcmp(cur_arg[0], "--all") == 0 ||
             strcmp(cur_arg[0], "-a") == 0) {
      unsetenv(ENV_FIRST_DEADLOCK);
      cur_arg++;
    }
    else if (strcmp(cur_arg[0], "--first-deadlock") == 0 ||
             strcmp(cur_arg[0], "--first") == 0 ||
             strcmp(cur_arg[0], "-f") == 0) {
      setenv(ENV_FIRST_DEADLOCK, "1", 1);
      cur_arg++;
    }
    else if (strcmp(cur_arg[0], "--check-for-livelock") == 0 ||
             strcmp(cur_arg[0], "-l") == 0) {
      setenv(ENV_CHECK_FOR_LIVELOCK, "1", 1);
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
    else if (strncmp(cur_arg[0], "--trace", strlen("--trace")) == 0 ||
             strncmp(cur_arg[0], "-t", strlen("-t")) == 0) {
      char *value;
      if (strcmp(cur_arg[0], "--trace") == 0 ||
          strcmp(cur_arg[0], "-t") == 0) {
        value = cur_arg[1];
        cur_arg += 2;
      } else if (strncmp(cur_arg[0], "--trace=", strlen("--trace=")) == 0) {
        value = cur_arg[0] + strlen("--trace=");
        cur_arg++;
      } else if (strncmp(cur_arg[0], "-t", strlen("-t")) == 0) {
        value = cur_arg[0] + strlen("-t");
        cur_arg++;
      }
      char *endptr;
      strtol(value, &endptr, 10);
      if (endptr[0] == '\0') { // if Single integer
        setenv(ENV_PRINT_AT_TRACE_ID, value, 1);
      } else if ( isdigit(*value) || isspace(*value) ) { // if array of integers
        setenv(ENV_PRINT_AT_TRACE_SEQ, value, 1);
      } else {
        fprintf(stderr, "%s: illegal value\n", "--trace");
        exit(1);
      }
    }
    else if (strcmp(cur_arg[0], "--quiet") == 0 ||
             strcmp(cur_arg[0], "-q") == 0) {
      setenv(ENV_QUIET, "1", 1);
      cur_arg++;
    }
    else if (strcmp(cur_arg[0], "--help") == 0 ||
             strcmp(cur_arg[0], "-h") == 0) {
      fprintf(stderr, "Usage: mcmini [--max-depth-per-thread|-m <num>]\n"
                      "              [--max-transitions-depth-limit|-M <num>]\n"
                      "                               (default num = %d)\n"
                      "              [--first-deadlock|--first|-f] (default)\n"
                      "              [--all-deadlocks|--all|-a]\n"
                      "              [--check-for-livelock|-l] (experimental)\n"
                      "              [--quiet|-q]\n"
                      "              [--trace|-t <num>|<traceSeq>]\n"
                      "              [--verbose|-v] [-v -v]\n"
                      "              [--help|-h]\n"
                      "              target_executable\n"
                      "       mcmini-gdb ...<same as mcmini args>...\n"
                      "       python3 mcmini-annotate.py -t <traceSeq> ...<same as mcmini args>...\n"
                      "       (To check data races in target, compile target as in\n" 
                      "        Makefile_llvm in the top level of the McMini source code.)\n",
              MC_STATE_CONFIG_MAX_TRANSITIONS_DEPTH_LIMIT_DEFAULT
             );
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
    exit(101); // Same as exit code 0145
  }

  assert(cur_arg[0][strlen(cur_arg[0])] == '\0');
  char idx = strlen(cur_arg[0]) - strlen("mcmini") - 1 >= 0 ?
             strlen(cur_arg[0]) - strlen("mcmini") - 1 :
             strlen(cur_arg[0]);
  // idx points to 'X' when cur_arg[0] == "...Xmcmini"
  if (strcmp(cur_arg[0], "mcmini") == 0 ||  strcmp(cur_arg[0] + idx, "/mcmini") == 0) {
    fprintf(stderr,
            "\n*** McMini being called on 'mcmini'.  This doesn't work.\n");
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
  // We execute target application as "template", and then fork traces.
  // McMini next appears as a constructor in mcmini_private.cpp:mcmini_main().
  // This is a constructor in libmcmini.so, loaded using LD_PRELOAD.
  execvp(cur_arg[0], cur_arg);
  fprintf(stderr, "Executable '%s' not found.\n", cur_arg[0]);
  perror("mcmini");
  return 1;
}
