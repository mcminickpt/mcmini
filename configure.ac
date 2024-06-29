#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([McMini],[1.0],[pirtle.m@northeastern.edu,jovanovic.l@northeastern.edu,gene@ccs.neu.edu],[mcmini],[https://github.com/mcminickpt/mcmini.git])
AC_CONFIG_SRCDIR([src/launch.c])
AC_CONFIG_HEADERS([include/mcmini/config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP

dnl AX_CXX_COMPILE_STDCXX([11], [noext], [mandatory])

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

AC_ARG_ENABLE([debug],
            [AS_HELP_STRING([--enable-debug],
                            [Use debugging flags "-Wall -g3 -O0"
                             (default is -g -O0)])],
            [use_debug=$enableval],
            [use_debug=no])

if test "$use_debug" = "yes"; then
  AC_SUBST([DEBUG], [yes])
  AC_DEFINE([DEBUG],[1],[Use debugging flags "-Wall -g3 -O0"])
  CFLAGS="$CFLAGS -Wall -g3 -O0 -DDEBUG"
  CPPFLAGS="$CPPFLAGS -Wall -g3 -O0 -DDEBUG"
  CXXFLAGS="$CXXFLAGS -Wall -g3 -O0 -DDEBUG"
else
  AC_SUBST([DEBUG], [no])
fi

AC_MSG_CHECKING([if python3 is available])
# If Python returns True, that is converted to '1', which is Failure for shell:
PYTHON=`which python 2> /dev/null`
PYTHON3=`which python3 2> /dev/null`
if [[ "$PYTHON" != "" ]] && \
    $PYTHON -c 'import sys; sys.exit(sys.version_info.major < 3)'; then
  has_python=yes
elif [[ "$PYTHON3" != "" ]] && \
    $PYTHON3 -c 'import sys; sys.exit(sys.version_info.major < 3)'; then
  PYTHON=`which python3`
  has_python=yes
else
  has_python=no
fi
AC_MSG_RESULT([$has_python])
if [[ $has_python == yes ]] && [[ $PYTHON != /usr/bin/python ]]; then
  # Replacing line 1 ( #!/usr/bin/env python ) with local python
  sed --in-place -e '1 s^/usr/.*$^'$PYTHON^ ./mcmini-annotate.py
fi
if [[ $has_python == no ]]; then
  AC_MSG_FAILURE("python3 not found")
fi

AC_CHECK_PROG([HAS_GDB],[gdb],[yes],[no])
if [[ $HAS_GDB == no ]]; then
  AC_MSG_FAILURE("gdb not found")
fi

# '[ [' must be doubled to escape the macro.
GDB_VERSION=`gdb --version | head -1 | sed -e 's%\([[^0-9]]*\)%%' -e 's%[[^0-9]].*$%%'`
PYTHON_VERSION_MINOR=`$PYTHON -c 'import sys; print("%d" % (sys.version_info.minor))'`

# If Python returns True, True is converted to '1', which is Failure for shell:
# FIXME: We should test if gdb.Frame.level is present, for GDB's need to patch.
#        This came into GDB somewhere from GDB-10 to GDB-12.
#        And capture_output flag came into Python3 in python 3.7.
# CentOS 8 still uses outdated Python and GDB.
if $PYTHON -c 'import sys; sys.exit('$GDB_VERSION' >= 12 and '$PYTHON_VERSION_MINOR' >= 7)'; then
  echo '*** Python or GDB Python API is older: patching Python code'
  echo '*** There may be some smaller missing features for mcminig-gdb.'
  # patch -p1 < mcmini-oldsys.patch
fi

dnl CMA (Cross Memory Attach) brought in with  Linux 3.2.0 and glibc 2.15
dnl Around Feb., 2018, Docker forbids process_vm_readv even though it's in
dnl   libc.so.  So, AC_CHECK_FUNC is not sufficient.  Use AC_RUN_IFELSE([AC_LANG_SOURCE([[]])],[],[],[]).
AC_MSG_CHECKING([if process_vm_readv/process_vm_writev (CMA) available])
dnl AC_CHECK_FUNC(process_vm_readv, [has_cma='yes'], [has_cma='no'])
AC_RUN_IFELSE([AC_LANG_SOURCE([[
       #define _GNU_SOURCE
       #include <sys/types.h>
       #include <unistd.h>
       #include <sys/wait.h>
       #include <sys/uio.h>

       int
       main(void)
       {
           struct iovec local[2];
           struct iovec remote[1];
           char buf1[10];
           char buf2[10];
           char remote_buf[100];
           ssize_t nread;
           pid_t childpid = fork();             /* PID of remote process */
           if (childpid > 0) { // if parent
             int status;
             int rc = waitpid(childpid, &status, 0);
             return WEXITSTATUS(status); // return child's return status
           }
           // else child
           int pid = getppid();

           local[0].iov_base = buf1;
           local[0].iov_len = 10;
           local[1].iov_base = buf2;
           local[1].iov_len = 10;
           remote[0].iov_base = (void *) remote_buf;
           remote[0].iov_len = 20;

           nread = process_vm_writev(pid, local, 2, remote, 1, 0);
           if (nread != 20)
               return 1;
           else
               return 0;
       }
       ]])],[has_cma='yes'],[has_cma='no'],[])
if test "$has_cma" == "yes"; then
  AC_DEFINE([HAS_PROCESS_VM],[1],[Define to 1 if you have process_vm_readv and process_vm_writev.])
  AC_SUBST([HAS_PROCESS_VM], [yes])
else
  AC_SUBST([HAS_PROCESS_VM], [no])
fi
AC_MSG_RESULT([$has_cma])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([mcmini-gdb], [chmod a+x mcmini-gdb])
AC_OUTPUT
