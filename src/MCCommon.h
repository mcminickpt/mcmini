#ifndef MCMINI_MCCOMMON_H
#define MCMINI_MCCOMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

int mcprintf(const char *format, ...);
void mcwrite(const char *str);
void mcflush();

#define memfill(x, y) memset(x, y, sizeof(x))
#define memzero(x) memfill(x, 0)

#endif //MCMINI_MCCOMMON_H
