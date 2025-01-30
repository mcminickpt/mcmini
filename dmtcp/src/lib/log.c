#include "mcmini/lib/log.h"
#include "mcmini/spy/intercept/interception.h"

#include <pthread.h>
#include <stdarg.h>
#include <sys/time.h>

static int global_log_level = MCMINI_LOG_MINIMUM_LEVEL;
static const char *log_level_strs[] = {
  "VERBOSE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "DISABLE"
};

typedef struct log_record {
  int level;
  int line;
  const char *file;
  const char *format;
  struct tm *time;
  va_list var_args;
  FILE *target;
} log_record;

void mcmini_log_set_level(int level) {
    static pthread_mutex_t level_mut = PTHREAD_MUTEX_INITIALIZER;
    libpthread_mutex_lock(&level_mut);
    global_log_level = level;
    libpthread_mutex_unlock(&level_mut);
}

void mcmini_log_toggle(bool enable) {
    mcmini_log_set_level(MCMINI_LOG_DISABLE);
}

void display_log_record(struct log_record *rc) {
  char buf[20];
  buf[strftime(buf, sizeof(buf), "%H:%M:%S", rc->time)] = '\0';
  fprintf(
    rc->target,
    "%s %-5s %s:%d: ",
    buf, log_level_strs[rc->level], rc->file, rc->line
  );
  vfprintf(rc->target, rc->format, rc->var_args);
  fprintf(rc->target, "\n");
  fflush(rc->target);
}

void mcmini_log(int level, const char *file, int line, const char *fmt, ...) {
    if (level < global_log_level) {
        return;
    }
    log_record rc;
    time_t t = time(NULL);
    rc.format = fmt;
    rc.file = file;
    rc.line = line;
    rc.level = level;
    rc.target = stdout;
    rc.time = localtime(&t);
    // va_start(rc.var_args, fmt);
    display_log_record(&rc);
    // va_end(rc.var_args);
}
