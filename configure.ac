#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([McMini], [1.0], [pirtle.m@northeastern.edu,jovanovic.l@northeastern.edu,gene@ccs.neu.edu])
AC_CONFIG_SRCDIR([src/launch.c])
# AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([atexit ftruncate setenv strtoul])

AC_SUBST(PWD)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([mcmini-gdb], [chmod a+x mcmini-gdb])
AC_OUTPUT
