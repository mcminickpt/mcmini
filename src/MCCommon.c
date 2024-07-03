#include "mcmini/MCCommon.h"

/* We want to allow GDB to temporarily set mcprintf_redirect to true,
 * so that mcprintf() does not immediately print to stdout.
 * A GDB script can then use:
 * gdb.execute("call mcprintf_redirect()")
 *   ...
 * gdb.execute("call mcprintf_stop_redirect()")
 * output = gdb.parse_and_eval("mcprintf_redirect_output").string()
 *   and later do:  (gdb) python print(output)
 *
 * The GDB variable max-value-size (default: 64 KB) determines
 * the maximum size of mcprintf_redirect_output[];
 */
static char mcprintf_redirect_output[10000];
#define NORMAL (-1)
static int mcprintf_idx = NORMAL;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static void mcprintf_redirect()
{
  mcprintf_idx = 0;
  mcprintf_redirect_output[0] = '\0';
}
static void mcprintf_stop_redirect() { mcprintf_idx = NORMAL; }
#pragma GCC diagnostic pop

int
mcprintf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  int ret = -1;
  if (mcprintf_idx == NORMAL) {
    ret = vprintf(format, args);
    mcflush();
  } else {
    if (mcprintf_idx >= sizeof mcprintf_redirect_output - 200) {
      snprintf(mcprintf_redirect_output + sizeof mcprintf_redirect_output - 200,
               200, "%s",
               "\n*** McMini: mcprintf_redirect_output full;"
               " Increase it and GDB max-value-size?\n");
    } else {
      ret = vsnprintf(mcprintf_redirect_output + mcprintf_idx,
                      sizeof mcprintf_redirect_output - mcprintf_idx,
                      format, args);
      mcprintf_idx += ret;
      if (ret >= sizeof mcprintf_redirect_output) { // then truncate:
        mcprintf_redirect_output[sizeof(mcprintf_redirect_output) - 1] = '\0';
      }
    }
  }
  va_end(args);
  return ret;
}

void
mcflush()
{
  fflush(stdout);
  fflush(stderr);
}

void
mcwrite(const char *str)
{}
