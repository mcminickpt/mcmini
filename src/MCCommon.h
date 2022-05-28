#ifndef MCMINI_MCCOMMON_H
#define MCMINI_MCCOMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

int mcprintf(const char *format, ...);
void mcwrite(const char *str);
void mcflush();

#endif //MCMINI_MCCOMMON_H
