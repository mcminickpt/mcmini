#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "MCEnv.h"

int
main(int argc, const char **argv)
{
    char **cur_arg = argv+1;
    if (argc == 1) { cur_arg[0] = "--help"; cur_arg[1] = NULL; }
    while (cur_arg[0] != NULL && cur_arg[0][0] == '-') {
        if (strcmp(cur_arg[0], "--max-trace-depth") == 0 || strcmp(cur_arg[0], "-m") == 0) {
            setenv(ENV_MAX_THREAD_DEPTH, cur_arg[1], 1);
            cur_arg += 2;
        } else if (strcmp(cur_arg[0], "--debug-at-trace") == 0 || strcmp(cur_arg[0], "-d") == 0) {
            setenv(ENV_DEBUG_AT_TRACE, cur_arg[1], 1);
            cur_arg += 2;
        } else if (strcmp(cur_arg[0], "--verbose") == 0 || strcmp(cur_arg[0], "-v") == 0) {
            setenv(ENV_VERBOSE, "1", 1);
            cur_arg++;
        } else if (strcmp(cur_arg[0], "--verbose") == 0 || strcmp(cur_arg[0], "-v") == 0) {
            setenv(ENV_STOP_AT_FIRST_DEADLOCK, "1", 1);
            cur_arg++;
        } else if (strcmp(cur_arg[0], "--help") == 0 ||
                   strcmp(cur_arg[0], "-h") == 0) {
            fprintf(stderr, "Usage: gmal [--max-trace-depth|-m <num>] "
                            "[--debug-at-trace|-d <num>]\n"
                            "[--first-deadlock|-first]\n"
                            "[--verbose|-v] [--help|-h] target_executable\n");
            exit(1);
        } else {
            printf("gmal: unrecognized option: %s\n", cur_arg[0]);
            exit(1);
        }
    }

    struct stat stat_buf;
    if (cur_arg[0] == NULL || stat(cur_arg[0], &stat_buf) == -1) {
        fprintf(stderr, "*** Missing target_executable or no such file.\n\n");
        exit(1);
    }

    char buf[1000];
    buf[sizeof(buf)-1] = '\0';
    // We add ours to the end of any PRELOAD of the target application.
    snprintf(buf, sizeof buf, "%s:%s/libgmalchecker.so",
             (getenv("LD_PRELOAD") ? getenv("LD_PRELOAD") : ""),
             dirname(argv[0]));
    // Guard against buffer overrun:
    assert(buf[sizeof(buf)-1] == '\0');
    setenv("LD_PRELOAD", buf, 1);
    execvp(cur_arg[0], cur_arg);
    fprintf(stderr, "Executable '%s' not found.\n", cur_arg[0]);
    perror("gmal");
    return 1;
}
