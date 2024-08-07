#ifndef INCLUDE_MCMINI_MCCOMMON_HPP
#define INCLUDE_MCMINI_MCCOMMON_HPP

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int mcprintf(const char *format, ...);
void mcwrite(const char *str);
void mcflush();
bool is_redirect_stdout(bool strip_newline);

#endif /* INCLUDE_MCMINI_MCCOMMON_HPP */
