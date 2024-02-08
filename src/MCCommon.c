#include "mcmini/MCCommon.h"

int
mcprintf(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  int ret = vprintf(format, args);
  mcflush();
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
