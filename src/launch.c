#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

int
main(int argc, const char **argv)
{
    char **cur_arg = argv+1;
    if (argc == 1) { cur_arg[0] = "--help"; cur_arg[1] = NULL; }
    if (strcmp(cur_arg[0], "--help") == 0 ||
        strcmp(cur_arg[0], "-h") == 0) {
        fprintf(stderr, "Usage: gmal [--help|-h] target_executable\n");
        exit(1);
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
    perror("mcmini");
    return 1;
}
