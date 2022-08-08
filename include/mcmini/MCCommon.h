#ifndef MCMINI_MCCOMMON_H
#define MCMINI_MCCOMMON_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int mcprintf(const char *format, ...);
void mcwrite(const char *str);
void mcflush();

#endif // MCMINI_MCCOMMON_H
