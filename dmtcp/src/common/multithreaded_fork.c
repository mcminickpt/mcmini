/*****************************************************************************
 * Copyright (C) 2024 Gene Cooperman <gene@ccs.neu.edu>                      *
 *                                                                           *
 * This program is free software: you can redistribute it and/or             *
 * modify it under the terms of the GNU Lesser General Public License as     *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * DMTCP is distributed in the hope that it will be useful,                  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Lesser General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with DMTCP.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/

// NOTE: This version of fork() does not support pthread_atfork(),
//        and it doesn't do the weird stuff about cancellation points,
//        or robust mutexes.

// Remove this 'define' if using this with other software.
// #define STANDALONE

// FIXME:  the virtual_to_real logic is not yet working, when not using libdmtcp.so
// Define this if using this with DMTCP:
// #define DMTCP

// FIXME:  glibc-2.28 and earlier don't define _Fork().  Replace by syscall and test.
// FIXME:  Check all SYS_gettid, SYS_tgkill, get_child_threads for whether
//         they use virtual or real tid; Probably extract declarations
//         from DMTCP, and then optionally test DMTCP.via dmtcp_is_enabled(),
//         or simply convert tall tids to virtual.
//         MODIFY: get_child_threads to return virtual tid.
//         NOTE:  syscall(SYS_tgkill, ...) takes a virtual tid.
// FIXME:  This is arguably a bug in glibc:
//         glibc returns EBUSY if pthread_join() is called and succeeds and pthread_tryjoin_np is then called.
//         It's because pthread_join() sets tid to -1, and then pthread_tryjoin_np tests 'if (tid != 0) {EBUSY;}'
//         It should return ESRCH rather than EBUSY if the child thread has already joined.
// FIXME:  Change _Fork to syscall(SYS_fork/SYS_clone, ...) (support all libc's)
//           and add comment about _Fork() for robust mutex if desired.
// FIXME:  After ARCH_SET_FS, errno becomes 22.  Why?
// FIXME:  We should clean up the temporary child thread stack after doing setcontext.
// This code still has to:
//   1. add the current tid to the thread descriptor (pthread_t)
//      DONE:  See getTLSPointer()/getTLSPointer()
//   2. set restore the TLS:  src/tls.cpp:TLSInfo_RestoreTLSPointer()
//      DONE:  See getTLSPointer(), setTLSPointer()
//   3. support pthread_sigmask
//      TODO:  See DMTCP:src/tls.cpp, near the call to setcontext()

#define _GNU_SOURCE
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h> // man 2 open
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ucontext.h>
#include <sched.h> // For clone()
#ifdef DMTCP
# include "dmtcp.h"

// FIXME:  These two macros should be in dmtcp.h, not here.
# define dmtcp_real_to_virtual_pid(PID) \
    (dmtcp_real_to_virtual_pid != NULL ? dmtcp_real_to_virtual_pid(PID) : 0)
# define dmtcp_virtual_to_real_pid(PID) \
    (dmtcp_virtual_to_real_pid != NULL ? dmtcp_virtual_to_real_pid(PID) : 0)
#endif

#ifdef MC_SHARED_LIBRARY
#include "mcmini/real_world/process/template_process.h"
#include "mcmini/spy/intercept/interception.h"
#endif

// These are for debugging, only.  If segfault, do infinite loop,
//   and later, we can attach with GDB.
#ifdef STANDALONE
static void SegvfaultHandler(int signum, siginfo_t *siginfo, void *context) {
  while(1);
}
static int AddSegvHandler() {
  struct sigaction act;
  static struct sigaction old_act;

  act.sa_sigaction = &SegvfaultHandler;
  act.sa_flags = SA_RESTART | SA_SIGINFO;
  sigemptyset(&act.sa_mask);
  if (sigaction(SIGSEGV, &act, &old_act)) {
    perror("Failed to install segv handler");
    return -1;
  }
  return 0;
}
#endif

// In DMTCP, 'struct threadinfo' is declared as DMTCP:src/threadinfo.h:ThreadTLSInfo
struct threadinfo {
  pid_t origTid; // Used for debugging, and 'origTid == 0'  if not a valid entry.
  ucontext_t context;
  // fs and gs only used in __x86_64__
  unsigned long fs;
  unsigned long gs;
  // tlsAddr only used in __aarch64__ and __riscv
  // In fact, __riscv has the address in a normal register, restored w/ context.
  unsigned long int tlsAddr;
  // The kernel has a process-wide sigmask, and also a per-thread sigmask.
  sigset_t thread_sigmask;
  // glibc:pthread_create and pthread_self use this, but not the clone call:
  pthread_t pthread_descriptor;
} childThread[1000];
int threadIdx = 0; // threadInfo[threadIdx] is 'struct threadinfo' for next thread.

#ifdef __x86_64__
# include <asm/prctl.h>
# include <sys/prctl.h>

void getTLSPointer(struct threadinfo *localThreadInfo) {

  assert(syscall(SYS_arch_prctl, ARCH_GET_FS, &localThreadInfo->fs) == 0);
  assert(syscall(SYS_arch_prctl, ARCH_GET_GS, &localThreadInfo->gs) == 0);
}

void setTLSPointer(struct threadinfo *localThreadInfo) {
  assert(syscall(SYS_arch_prctl, 2, ARCH_SET_FS, localThreadInfo->fs) != 0);
  assert(syscall(SYS_arch_prctl, 2, ARCH_SET_GS, localThreadInfo->gs) != 0);
}
#elif defined(__aarch64__)
// 1776 valid for glibc-2.17 and beyond
static void getTLSPointer(struct threadinfo *localThreadInfo) {
  unsigned long int addr;
  asm volatile ("mrs   %0, tpidr_el0" : "=r" (addr));
  localThreadInfo->tlsAddr = addr - 1776;  // sizeof(struct pthread) = 1776
}
static void setTLSPointer(struct threadinfo *localThreadInfo) {
  unsigned long int addr = localThreadInfo->tlsAddr + 1776;
  asm volatile ("msr     tpidr_el0, %[gs]" : :[gs] "r" (addr));
}
#elif defined(__riscv)
void getTLSPointer(struct threadinfo *localThreadInfo)
{
  unsigned long int addr;
  asm volatile ("addi %0, tp, 0" : "=r" (addr));
  localThreadInfo->tlsAddr = addr - 1856;  // sizeof(struct pthread)=1856
}
void setTLSPointer(struct threadinfo *localThreadInfo)
{
  unsigned long int addr = localThreadInfo->tlsAddr + 1856;
  asm volatile("addi tp, %[gs], 0" : : [gs] "r" (addr));
}
#else
void getTLSPointer(struct threadinfo *localThreadInfo) {}
void setTLSPointer(struct threadinfo *localThreadInfo) {}
#endif

// SEE DMTCP:src/tls.cpp:TLSInfo_GetTidOffset()
//   and DMTCP:src/tls.cpp:TLSInfo_RestoreTLSTidPid(Thread *thread)
// If constants are not right, run DMTCP:util/check-pthread-tid-offset.c -v
// Returns offset of tid in pthread_t.
int pthreadDescriptorTidOffset() {
#ifdef __x86_64__
  // Since glibc-2.11:
  int offset = 720;  // sizeof(tcbhead_t) + sizeof(list_t)
#elif defined(__aarch64__)
  int offset = 208;
#elif defined(__riscv)
  int offset = 208;
#endif
  return offset;
}
pid_t get_tid_from_pthread_descriptor(pthread_t pthread_descriptor) {
  int offset = pthreadDescriptorTidOffset();
  pid_t ctid = *(pid_t*)((char*)(pthread_descriptor) + offset);
#ifdef DMTCP
  pid_t virttid = dmtcp_real_to_virtual_pid(ctid);
  ctid = (virttid ? virttid : ctid);
#endif
  return ctid;
}
static pid_t patchThreadDescriptor(pthread_t pthreadSelf) {
  int offset = pthreadDescriptorTidOffset();
  pid_t oldtid = *(pid_t *)((char *)pthreadSelf + offset);
  // Since glibc.2.25, tid, but not pid, is stored in pthread_t.
  // gettid() supported only in glibc-2.30; So, we use syscall().
  *(pid_t *)((char *)pthreadSelf + offset) = syscall(SYS_gettid);
  return oldtid;
}

// Semaphore for parent thread.
// Used in the original process before fork, and child process after fork.
sem_t sem_fork_parent;
// Semaphore for child thread; Only used in child process.
sem_t sem_fork_child;

void restart_child_threads();
void multithreaded_fork_child_handler(int sig);

#define SIG_MULTITHREADED_FORK (SIGRTMIN+6)

int get_child_threads(int child_threads[]) {
    DIR *dir;
    struct dirent *entry;
    dir = opendir("/proc/self/task");
    if (dir == NULL) {
      perror("opendir");
      return -1;
    }
    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
      if (atoi(entry->d_name) != 0) {
        pid_t nexttid = atoi(entry->d_name);
#ifdef DMTCP
        pid_t virttid = dmtcp_real_to_virtual_pid(nexttid);
        nexttid = (virttid ? virttid : nexttid);
#endif
        child_threads[i++] = nexttid;
      }
    }
    child_threads[i] = 0;
    closedir(dir);
    return i;
}

// Optimization: could skip pthread_sigmask for targets not using it
//   (We would need a wrapper function to test it.)
void saveThreadStateBeforeFork(struct threadinfo* threadInfo) {
  threadInfo->origTid = syscall(SYS_gettid);

  // For verification only; not needed for functionality
  pid_t oldTid = patchThreadDescriptor(pthread_self());
  if (oldTid != syscall(SYS_gettid)) {
    fprintf(stderr, "PID %d: multithreaded_fork(): patchThreadDescriptor:"
         "bad offset:\n        Run: DMTCP:util/check-pthread-tid-offset.c\n",
         getpid());
    abort();
  }
  threadInfo->pthread_descriptor = pthread_self();
  getTLSPointer(threadInfo);

  // FIXME:  Add func fo get/set signals in child thread of child process.
  //         and restore thread sigmask sfter setcontext.
  pthread_sigmask(SIG_BLOCK, NULL, &threadInfo->thread_sigmask);
  sigset_t sigtest;
  pthread_sigmask(SIG_BLOCK, NULL, &sigtest);
  sigdelset(&sigtest, SIG_MULTITHREADED_FORK);
  if (! sigisemptyset(&sigtest)) {
    fprintf(stderr, "PID %d: multithreaded_fork() not yet implemented"
                    " for non-empty thread signaks\n", getpid());
    abort();
  }
}

void restoreThreadStateAfterFork(struct threadinfo* threadInfo) {
  setTLSPointer(threadInfo); // Set the fs register (set thread-local address)
// FIXME:
// NOTE:  glibc clone allocated a new pthread descriptor on the temporary stack used by child_setcontext
//        But the tcb (Thread Control Block) always starts at %fs:0 (or tpidr_el0 for aarch64 or tp for riscv).
//        And glibc finds the current pthread_descriptor as pointed to by %fs:0x10
//        Now that we have restored %fs, we can restore the pthread descriptor.
//        Caveat:  I hope that the %fs used in glibc clone didn't overwrite our restored %fs and
//          then overwrite the tcb.  If it did, we would have to restore the full tcb.
//        QUESTION:  Does glibc fork() keep the same %fs:0 address in child?
//  NOTE:  pthread_tryjoin_np() was signaling an error, until we did this.
//         But pthread_join() has an error: errno 2: No such file or directory  IN:
//           rc1 error: perror("****** Child process: parent thread: pthread_join");
  patchThreadDescriptor(threadInfo->pthread_descriptor); // Update the tid
}


# define STRINGIFY2(x) #x
# define STRINGIFY(x) STRINGIFY2(x)
void signal_multithreaded_fork_handler() {
  sigset_t emptyset;
  sigemptyset(&emptyset);
  struct sigaction new = {.sa_handler = multithreaded_fork_child_handler,
                          .sa_flags = SA_RESTART, .sa_mask = emptyset};
  struct sigaction old;
  sigaction(SIG_MULTITHREADED_FORK, &new, &old);
  if (old.sa_handler != SIG_DFL) {
    fprintf(stderr,
            "\n***************************************************************\n"
              "* WARNING:  There was a previous handler for signal %s\n"
              "*           And multithreaded_fork() is overwriting it.\n"
              "*           Maybe change SIG_MULTITHREADED_FORK in source code.\n"
              "***************************************************************\n\n",
            STRINGIFY(SIG_MULTITHREADED_FORK));
  }
}

pid_t multithreaded_fork() {
  static int initialized = 0;
  if (! initialized) {
    sem_init(&sem_fork_child, 0, 0);
    sem_init(&sem_fork_parent, 0, 0);
    threadIdx = 0; // Needed for multiple calls to multithreaded_fork()
    initialized = 1;
  }

  static int handler_is_declared = 0;
  if (! handler_is_declared) {
    signal_multithreaded_fork_handler();
    handler_is_declared = 1;
  }

  int child_threads[1000];
  int num_threads = get_child_threads(child_threads);
  const pid_t mytid = syscall(SYS_gettid);

#ifdef MC_SHARED_LIBRARY
  const pid_t ckpt_tid = get_tid_from_pthread_descriptor(ckpt_pthread_descriptor);
#endif

  for (int i = 0; i < num_threads; i++) {

    // FIXME: We don't have to check `mytid` below since we check it
    // here already.
    if (child_threads[i] == mytid) {
      childThread[i].origTid = 0;
      continue;
    }

#ifdef MC_SHARED_LIBRARY
    if (child_threads[i] == ckpt_tid) {
      childThread[i].origTid = 0;
      continue;
    }
#endif
#ifdef DMTCP
    if (child_threads[i] != mytid && child_threads[i] != ckpt_tid) {
#else
    if (child_threads[i] != mytid) {
#endif
      syscall(SYS_tgkill, getpid(), child_threads[i], SIG_MULTITHREADED_FORK);
    }
  }

#ifdef DMTCP
  // NOTE: `(num_threads - 2) =  # user space threads - this thread (1) - checkpoint thread (1)
  int num_secondary_threads = num_threads - 2;
#else
  // NOTE: `(num_threads - 1) =  # user space threads - this thread (1)
  int num_secondary_threads = num_threads - 1;
#endif

  for (int i = 0; i < num_secondary_threads; i++) {
    sem_wait(&sem_fork_parent); // Wait until children have initialized context.
  }

  /*********************************************************************
   *NOTE:  We want to skip the normal fork cleanup
   *       of child threads (e.g., reclaim_stack()).
   *       So, we call an internal version of fork().
   *
   *__libc__fork() calls:
   *
   *../sysdeps/unix/sysv/linux/arch-fork.h:26
   *__libc_fork() calls:
   *_Fork() {
   *  pid_t pid = arch_fork (&THREAD_SELF->tid);
   *  ... robust mutex support ...
   *}
   *
   *../sysdeps/unix/sysv/linux/arch-fork.h:34
   *statinc inline pid_t arch_fork(void *ctid) {
   *  echo 'flags = CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID | SIGCHLD'
   *  ret = INLINE_SYSCALL_CALL (clone, flags, 0, NULL, ctid, 0);
   *}
   *********************************************************************/
#if 1
  pid_t _Fork();
  int childpid = _Fork();
#else
  // NOT YET FULLY DEVELOPED:
  int flags = CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID | SIGCHLD;
  int childpid;
// syscall(SYS_clone, ...);
// stack must be NULL
// https://stackoverflow.com/questions/2898579/clone-equivalent-of-fork
//   But that says to use only SIGCHLD for flags, and glibc uses the above.
//   But it's okay, since we're setting ctid and tls to NULL.
// FIXME:  If we're going to set the last 3 args to NULL, who cares in what order they're found!
# ifdef __x86_64__
           long clone(unsigned long flags, void *stack,
                      int *parent_tid, int *child_tid,
                      unsigned long tls);
# elif defined(__aarch64__)
           long clone(unsigned long flags, void *stack,
                     int *parent_tid, unsigned long tls,
                     int *child_tid);
# elif defined(__riscv)
#  error Unimplemented CPU architecture
https://github.com/bminor/glibc/blob/master/sysdeps/unix/sysv/linux/riscv/clone.S
int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
	     void *parent_tidptr, void *tls, void *child_tidptr) */
	/* The syscall expects the args to be in different slots.  */
	mv		a0,a2
	mv		a2,a4
	mv		a3,a5
	mv		a4,a6
# endif
#endif
  initialized = 0;  // Reset for next call to multithreaded_fork()

  if (childpid > 0) { // if parent process
    for (int i = 0; i < num_secondary_threads; i++) {
      sem_post(&sem_fork_child);
    }
  } else { // else if child process

    restart_child_threads(num_threads);
    for (int i = 0; i < num_secondary_threads; i++) {
      sem_post(&sem_fork_child);
    }
    // Wait until child thread posts to us before leaving handler.
    for (int i = 0; i < num_secondary_threads; i++) {
      sem_wait(&sem_fork_parent);
    }
  }
  return childpid;
}

// This could be optimized for performance:
//   orig_pid could be static var, get/setTLSPointer could use assembly (cf MANA)
void multithreaded_fork_child_handler(int sig) {
  if (sig == SIG_MULTITHREADED_FORK) {
    // The handler is called before fork().  So, this will always be parent pid
    pid_t orig_pid = getpid();
    int origThreadIdx = __sync_fetch_and_add(&threadIdx, 1);
    struct threadinfo* threadInfo = &childThread[origThreadIdx];
    // We could set next origThreadIdx to 0, but another thread might it
    // childThread[origThreadIdx+1].origTid = 0;

    saveThreadStateBeforeFork(threadInfo);

    assert(getcontext(&threadInfo->context) == 0);
    // setcontext() returns to here after fork() and clone() of
    //   child thread (setcontext) and setTLSPointer()

    sem_post(&sem_fork_parent); // Before fork, to parent thread: did getctxt()
    if (getpid() != orig_pid) { // if we forked (if we are child process)
      // Child thread did setcontext and returned above into getcontext.
      // Let's post that we, the child thread, now exist.
      // Then we, the child thread, will wait on sem_fork_child until
      // the parent thread of the child process posts to us.
      sem_post(&sem_fork_parent);
    }
    sem_wait(&sem_fork_child);
  }
}

#if 0
#include <linux/futex.h>
#include <stdint.h>
#include <sys/time.h>
#endif
// 'int' return type required by 'clone()'
int child_setcontext(void *arg) {
  struct threadinfo* threadInfo = arg;
  // restoreThreadStateAfterFork(threadInfo);
  setTLSPointer(threadInfo);
  patchThreadDescriptor(threadInfo->pthread_descriptor);
  setcontext(&(threadInfo->context));
  return 0; // not reached
}

void restart_child_threads(int num_threads) {
  for (int i = 0; i < num_threads; i++) {
    if (childThread[i].origTid == 0) continue;

    // int clone(int (*fn)(void *), void *stack, int flags, void *arg, ...
    //           /* pid_t *parent_tid, void *tls, pid_t *child_tid */ );
    int clone_flags = (CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SYSVSEM
                       | CLONE_SIGHAND | CLONE_THREAD
                       | CLONE_SETTLS | CLONE_PARENT_SETTID
                       | CLONE_CHILD_CLEARTID
                       | 0);
    // This is a temporary stack, to be replaced after setcontext.
    // FIXME: This stack is a memory leak.  We should free it later.
    void *stack = malloc(0x10000) + 0x10000 - 128; // 64 KB
    int offset = pthreadDescriptorTidOffset();
    pid_t *ctid = (pid_t*)((char*)childThread[i].pthread_descriptor + offset);
    pid_t *ptid = ctid;
    // For more insight, read 'man set_tid_address'.
    clone(child_setcontext,
                      stack,
                      clone_flags,
                      (void *)&childThread[i], ptid, childThread[i].fs, ctid);
  }
}

// ===========================================================================
// This main routine and child() demonstrates the use of multithreaded_fork().
#ifdef STANDALONE

sem_t sem_parent;
sem_t sem_child;

void fprintf_thread(FILE *stream, const char *format_string, int pid) {
  char buf[500];
  snprintf(buf, sizeof(buf), format_string, pid);
  // This isn't really thread-safe, but kernel usually finishes this
  // syscall before starting a new syscall.
  write(fileno(stream), buf, strlen(buf));
}
void printf_thread(const char *format_string, int pid) {
  fprintf_thread(stdout, format_string, pid);
}

void *child_thread(void *dummy) {
printf("=============== 3. (char *)pthread_self()+720: %p\n", (char *)pthread_self()+720);
  int orig_pid = getpid(); // We do this before forking; So, it's the parent.
  pthread_t orig_pthread_self = pthread_self();
  sem_post(&sem_parent);
  int rc = sem_wait(&sem_child); // Wait while parent does multithreaded fork()
  // Parent has done multithreaded_fork();  we are in child or parent process
  if (rc != 0) {
    perror("sem_wait");
  }
  // 'errno' is a thread-local variable.  Test if thread-local was restored.
  int fd = open("/FILE_DOES_NOT_EXIST", O_RDONLY);
  if (getpid() != orig_pid) {
    assert(fd == -1 && errno == ENOENT);
  }
  // We have now forked.  If we are a child thread of the child process, do we
  // still remember our orig_pthread_self (from child thread
  // of parent process)?  Test if it's the same.
  if (getpid() != orig_pid) {
    assert(pthread_equal(pthread_self(), orig_pthread_self));
  }
if (getpid() != orig_pid) {
printf("=============== 4. (char *)pthread_self()+720: %p\n", (char *)pthread_self()+720);
printf("=============== 4b. *(int *)((char *)pthread_self()+720): %d\n", *(int *)((char *)pthread_self()+720));
printf("=============== 5. (char *)pthread_self()+720: %p\n", (char *)pthread_self()+720);
printf("=============== 5b. *(int *)((char *)pthread_self()+720): %d\n", *(int *)((char *)pthread_self()+720));
}
  if (getpid() != orig_pid) {
    sleep(1); // Let's see if parent or child process joins with this thread.
  }
  char format[] = "Child thread %ld of %s process is exiting.";
  char *return_string = malloc(50);
  snprintf(return_string, 50, format, syscall(SYS_gettid),
    (getpid() == orig_pid ? "parent" : "child"));
  return return_string;
}

#pragma GCC diagnostic ignored "-Wformat-security"
#ifndef NUM_THREADS
# define NUM_THREADS 3
#endif
int main() {
  sem_init(&sem_parent, 0, 0);
  sem_init(&sem_child, 0, 0);
AddSegvHandler();
  pthread_t thread[NUM_THREADS-1];
  for (int i = 0; i < NUM_THREADS-1; i++) {
    pthread_create(&thread[i], NULL, child_thread, NULL);
  }
  for (int i = 0; i < NUM_THREADS-1; i++) {
    sem_wait(&sem_parent);
  }
printf("=============== 2. (char *)thread[0]+720: %p\n", (char *)thread[0]+720);
  printf("Forking ...\n");
  pid_t childpid = multithreaded_fork();
  for (int i = 0; i < NUM_THREADS-1; i++) {
    sem_post(&sem_child); // Let child thread move on.
  }

  if (childpid > 0) { // If parent process (parent thread)
    // Wait and allow parent thread of child process to do pthread_join first.
    int status;
    assert(waitpid(childpid, &status, 0) == childpid);
    printf("waitpid: child process has exited %s.\n",
           (WIFEXITED(status) ? "normally" : "**ABNORMALLY**"));
    if (WEXITSTATUS(status) != 0) {
      printf("*** The child process had exited with return code %d\n",
             WEXITSTATUS(status));
    }
    if (WIFEXITED(status) == 0 && WIFSIGNALED(status)) {
      printf("*** The child process had terminated with the signal %d\n",
             WTERMSIG(status));
    }
  }

if (childpid == 0) {
printf("=============== 6. (char *)thread[0]+720: %p\n", (char *)thread[0]+720);
}
  void *child_return_string;
  for (int i = 0; i < NUM_THREADS-1; i++) {
    assert(pthread_join(thread[i], &child_return_string) == 0);
    printf("Child thread has joined %ld: %s\n",
                  syscall(SYS_gettid), (char *)child_return_string);
    free(child_return_string); // Was malloc'ed in child thread
  }

  if (childpid == 0) { // if child process (parent thread)
    // NOTE: glibc:pthread_tryjoin_np has a bug.  If pthread_join is called
    //   first, then the tid field of 'struct pthread' will be '-1',
    //   and so pthread_tryjoin_np returns EBUSY, instead of ESARCH.
    int rc2 = pthread_tryjoin_np(thread[0], &child_return_string);
    // non-posix:  Returns EBUSY or 0 or thread_tid, below, == -1
    //   But it never returns eSRCH (no such thread).
    int offset = pthreadDescriptorTidOffset();
    pid_t thread_tid = *(int *)((char *)thread[0] + offset);
    if (rc2 == EBUSY && thread_tid != -1) {
      fprintf_thread(stderr,
                     "\n*** ERROR: PID %d: pthread_tryjoin_np detected"
                     " EXTRA thread\n***        (bug in pthread_t:tid?)\n"
                     "*** NOTE: The 'pthread_join' likely returned success.\n"
                     "***       without joining to any child thread.\n"
                     "***       Run: DMTCP:util/check-pthread-tid-offset.c\n\n",
                    getpid());
      fflush(stderr);
      return 0;
    } else if (rc2 == EINVAL) {
      printf_thread("*** PID %d: pthread_tryjoin_np returned EINVAL\n",
                    getpid());
    } else {
      printf_thread("PID %d: And pthread_tryjoin_np shows that there are"
                    " no more child threads.\n", getpid());
    }
  }

  return 0;
}

#endif

// ===========================================================================
// DOCUMENTATION NOTES:
#if 0
glibc uses a pthread descriptor (often noted as 'struct pthread *pd' in the code).
The user code sees this as 'pthread_t', which is the same as 'struct pthread *'.
Therefore, pthread_self() returns a pointer to the thread descriptor of the current thread.

When we call clone() in glibc, glibc tries to place a newly allocated
  thread descriptor at the top of the stack that was passed to clone.
The relevant codes in glibc are:
  GLIBC/nptl/descr.h - defines 'struct pthread'. The first field is 'tcbhead_t header'
  GLIBC/sysdeps/x86_64/nptl/tls.h - defines  'tcbhead_t', and also THREAD_SELF
  GLIBC/sysdeps/x86_64/nptl/tcb-access.h - defines THREAD_GETMEN and THREAD_SETMEM
  GLIBC/nptl/pthread_create.c - creates new thread descriptor, and calls clone()

The first three fields of tcbhead_t are:
  void *tcb;            /* Pointer to the TCB.  Not necessarily the
                           thread descriptor used by libpthread.  */
  dtv_t *dtv;
  void *self;           /* Pointer to the thread descriptor.  */

THREAD_SELF is defined as:
  #  define THREAD_SELF \
     (*(struct pthread *__seg_fs *) offsetof (struct pthread, header.self))
where the macro is referring to the 'header' field at the beginning of 'struct pthread'.
In fact, pthread_self(), internally, is simply returning THREAD_SELF.
NOTE ON THREAD_SELF:  Apparently, the notation '*__seg_fs' tells the compiler to use the
                      %fs segment register.  So, pthread_self() executes 'mov    %fs:0x10,%rax' and returns %rax.
                      where %fs:0 is pointing to header (tcbhead_t field of pthread descriptor).
                      Note that 0x10 is the offset of 'self' in tcbhead_t, thus confirming our analysis.
  So, in conclusion, at assembly level, THREAD_SELF will alwasy translate to %fs:0x10,
  which is always a pointer to the pthread_t thread descriptor.
  Thus, if we wanted to change the thread to point to a different thread descriptor, it is as simple as:
    void *tmp[] = (void **)pthread_self();  tmp[2] = address_of_new_thread_descriptor;
    (But only after verifying that your fs register is correct.)

The dtv field is a data structure to access variables across multiple dynamic libraries.
It is referred to here in case there are thread-local variables declared in dynamic variables.
We are not concerned with it here, but dtv stands for "Dynamic Thread Vector",
and there is a good description here:
  https://chao-tic.github.io/blog/2018/12/25/tls
  "A Deep dive into (implicit) Thread Local Storage"

The glibc internal code does not access the thread descriptor fields directly.
Instead, it uses THREAD_GETMEN and THREAD_SETMEM.  Almost always, it is invoked as:
  THREAD_GETMEN(THREAD_SELF, name_of_descriptor_field)
  THREAD_SETMEN(THREAD_SELF, name_of_descriptor_field, new_value)

QUESTION:
clone() with CLONE_SETTLS -- It says this can be used to set %fs register.  True?

COMMENT:
  GLIBC/sysdeps/x86_64/nptl/tls.h has a comment:
    /* Must be kept even if it is no longer used by glibc since programs,
       like AddressSanitizer, depend on the size of tcbhead_t.  */
  Hence, GLIBC is unlikely to change the size of tcbhead_t in the future.
  This implies, luckily for us, that the offset of tid in pthread_t is also
  not likely to change, in the future.

NOTE:
  'man set_tid_address' has important information about CLONE_CHILD_CLEARTID.
  In particular, the kernel keeps a clear_child_tid address associated with
    the child thread created by clone().  This points to the 'tid' field
    of the 'struct pthread' (of the struct pointed to by 'pthread_t').
    When pthread_join wakes from FUTEX_WAKE, it sets 'tid' to -1.
#endif
