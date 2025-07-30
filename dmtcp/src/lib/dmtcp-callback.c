#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "dmtcp.h"
#include "mcmini/mcmini.h"

// We probably won't need the '#undef', but just in case a .h file defined it:
#undef dmtcp_mcmini_is_loaded
int dmtcp_mcmini_is_loaded(void) { return 1; }
int counter = 0;

pthread_t ckpt_pthread_descriptor;
volatile atomic_bool libmcmini_has_recorded_checkpoint_thread;
static sem_t template_thread_sem;
static pthread_cond_t template_thread_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t template_thread_mut = PTHREAD_MUTEX_INITIALIZER;

bool is_checkpoint_thread(void) {
  return pthread_equal(pthread_self(), ckpt_pthread_descriptor);
}

void thread_handle_after_dmtcp_restart(void) {
  // IMPORTANT: There's a potential race between
  // notifying the template thread and accessing
  // the new current mode. We care about how
  // we're supposed to react after restarting. The template
  // thread, once signaled by all userspace threads, will
  // change its mode to `TARGET_TEMPLATE_AFTER_RESTART`.
  enum libmcmini_mode mode_on_entry = get_current_mode();
  notify_template_thread();
  switch (mode_on_entry) {
    case DMTCP_RESTART_INTO_TEMPLATE: {
      // For userspace threads in the template process, the threads
      // must wait _forever_. Since these userspace threads will eventually
      // become active and start listening to the model checker in the
      // multithreaded fork process, we need to be able to control
      // when these threads should begin listening to the model checker.
      // This is accomplished by using the current "mode" of libmcmini:
      // as long as we're not in the `TARGET_BRANCH_AFTER_RESTART` case, we
      // simply ignore any wakeups
      libpthread_mutex_lock(&template_thread_mut);
      while (get_current_mode() != TARGET_BRANCH_AFTER_RESTART) {
        libpthread_cond_wait(&template_thread_cond, &template_thread_mut);
      }
      libpthread_mutex_unlock(&template_thread_mut);
      break;
    }
    case DMTCP_RESTART_INTO_BRANCH: {
      // In the case of calling `dmtcp_restart` fpr each branch, we
      // are expected to immediately talk to the model checker.
      log_debug("Restarting into branch (DMTCP_RESTART_INTO_BRANCH)\n");
      break;
    }
    default: {
      // If we're in any other mode other than restarting at this point,
      // this is a bug. Note the potential race highlighted above that is
      // mitigated with the local variable.
      //
      // However, this solution assumes that the template thread indeed waits
      // until all userspace threads have notified it. Make sure that the logic
      // for the template thread matches this.
      libc_abort();
    }
  }

  // Finally, we can communicate directly with the model checker `mcmini`.
  thread_await_scheduler();
}

void mc_template_thread_loop_forever(void);
static void *template_thread(void *unused) {
  // Phase 1. The template thread is created at record-time
  // _prior_ to checkpoint. It must not intervene until exclusively
  // after a `dmtcp_restart`.
  //
  // This is accomplished by waiting on a semaphore that is only incremented
  // once the `DMTCP_EVENT_RESTART` is sent to `libmcmini.so`.
  libpthread_sem_wait_loop(&template_thread_sem);

  // Phase 2. Wait for all userspace threads to move into a stable state
  //
  // After the `DMTCP_EVENT_RESTART` event is send to `libmcmini.so`, all
  // userspace threads will resume execution. These userspace threads ,
  //
  // TODO: Even in the event in which we don't need to transfer the records
  // during the RECORD phase, we still wait for the userspace threads to go into
  // the "stable" state. This is probably not needed. Indeed, for the
  // `DMTCP_RESTART_INTO_BRANCH` case, waiting after each `dmtcp_restart` call
  // to ensure a stable recorded state is pointless: we're not going to read it
  // anyway! This is an only a potential optimization for later though.

  int thread_count = 0;
  struct dirent *entry;
  DIR *dp = opendir("/proc/self/task");
  if (dp == NULL) {
    perror("opendir");
    mc_exit(EXIT_FAILURE);
  }

  while ((entry = readdir(dp)))
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      thread_count++;

  // We don't want to count the template thread nor
  // the checkpoint thread, but these will appear in
  // `/proc/self/tasks`
  thread_count -= 2;
  closedir(dp);
  log_debug(
      "There are %d threads... waiting for them to get into a consistent "
      "state...\n",
      thread_count);
  for (int i = 0; i < thread_count; i++) {
    libpthread_sem_wait_loop(&dmtcp_restart_sem);
  }
  log_debug("The threads are now in a consistent state\n");

  if (get_current_mode() == DMTCP_RESTART_INTO_BRANCH) {
    atexit(&mc_exit_main_thread_in_child);
    set_current_mode(TARGET_BRANCH_AFTER_RESTART);
  }

  volatile struct mcmini_shm_file *shm_file = global_shm_start;
  volatile struct template_process_t *tpt = &shm_file->tpt;
  pid_t target_branch_pid = dmtcp_virtual_to_real_pid(getpid());
  tpt->cpid = target_branch_pid;
  libpthread_sem_post((sem_t *)&tpt->mcmini_process_sem);

  if (get_current_mode() == DMTCP_RESTART_INTO_TEMPLATE) {
    log_debug("Restarting into template (DMTCP_RESTART_INTO_TEMPLATE)\n");
    mc_template_thread_loop_forever();

    // Reaching this point means that we're in the branch: the
    // parent process (aka the template) will never exit
    // the above call to `mc_template_process_loop_forever()`.
    set_current_mode(TARGET_BRANCH_AFTER_RESTART);

    // Recall that the userspace threads in the template process
    // were idling/doing nothing. Indeed, those threads exist ONLY
    // to ensure that `multithreaded_fork()` clones them.
    //
    // Now that we're finally in the branch, we can signal each
    // of these userspace threads that checkpointing has begun.
    libpthread_mutex_lock(&template_thread_mut);
    libpthread_cond_broadcast(&template_thread_cond);
    libpthread_mutex_unlock(&template_thread_mut);
  }

  // Phase 3. Once in a stable state, check if `mcmini` needs to construct
  // a model of what we've recorded.
  if (getenv("MCMINI_NEEDS_STATE")) {
    log_debug("The template thread is transferring state");

    // FIXME: There appears to be an issue with opening the FIFO
    // here. If it already exists most likely it should be replaced,
    // but we seem to block on both sides (the `McMini` process side and here)
    // or else exit with `No file or directory`. It's probably a race.
    //
    // The current work around is to simply remove the named FIFO manually
    // and run a few exections until the race "resolves" itself (just hope that
    // they don't block).
    int rc = mkfifo("/tmp/mcmini-fifo", S_IRUSR | S_IWUSR);
    if (rc != 0) {
      perror("mkfifo");
    }
    int fd = open("/tmp/mcmini-fifo", O_WRONLY);
    if (fd == -1) {
      perror("open");
      mc_exit(EXIT_FAILURE);
    }
    for (rec_list *entry = head_record_mode; entry != NULL;
         entry = entry->next) {
      if (entry->vo.type == MUTEX) {
        log_verbose("Writing mutex entry %p (state %d)\n", entry->vo.location,
               entry->vo.mut_state);
      } else if (entry->vo.type == THREAD) {
        log_verbose("Writing thread entry %p (id %d, status: %d)\n",
               (void *)entry->vo.thrd_state.pthread_desc,
               entry->vo.thrd_state.id, entry->vo.thrd_state.status);
      } else if (entry->vo.type == CONDITION_VARIABLE) {
        log_verbose("Writing condition variable entry %p (status %d) (count %d) (waiting_queue %p)\n",
               entry->vo.location, entry->vo.cond_state.status, entry->vo.cond_state.waiting_threads->size,
               entry->vo.cond_state.waiting_threads);
      } else if (entry->vo.type == SEMAPHORE) {
        log_verbose("Writing semaphore entry %p (count %d, status: %d)\n",
               (void *)entry->vo.location,
               entry->vo.sem_state.count, entry->vo.sem_state.status);
      } else {
        libc_abort();
      }
      if (entry->vo.type == CONDITION_VARIABLE) {
        // Here we are trying to modify the variables that are a part of union,
        // so doing so is not ideal because it will change other varibale as memory is overlapped.
        thread_queue_node* current = entry->vo.cond_state.waiting_threads->front;
        while(current != NULL){
          visible_object waiting_queue_vo = {
            .type = CV_WAITERS_QUEUE,
            .location = current,
            .waiting_queue_state.cv_location = entry->vo.location,
            .waiting_queue_state.waiting_id = current->thread,
            .waiting_queue_state.cv_state = current->thread_cv_state
          };
          int sz = write(fd, &waiting_queue_vo, sizeof(visible_object));
          assert(sz == sizeof(visible_object));
          current = current->next;
        }
        int sz = write(fd, &entry->vo, sizeof(visible_object));
        assert(sz == sizeof(visible_object));
      }
      else {
        int sz = write(fd, &entry->vo, sizeof(visible_object));
        assert(sz == sizeof(visible_object));
      }
    }
    int sz = write(fd, &empty_visible_obj, sizeof(empty_visible_obj));
    assert(sz == sizeof(visible_object));
    fsync(fd);
    fsync(0);
    close(fd);
  }

  // Exiting from the template thread is fine:
  // once we're in the target branch, we no longer care
  // about it anyways.
  //
  // NOTE: This is true for both repeated `dmtcp_restart` AND for multithreaded
  // forking.
  log_debug("The template thread has completed: exiting...");
  return NULL;
}
// static void SegvfaultHandler(int signum, siginfo_t *siginfo, void *context) {
//   while(1);
// }
// static int AddSegvHandler() {
//   struct sigaction act;
//   static struct sigaction old_act;

//   act.sa_sigaction = &SegvfaultHandler;
//   act.sa_flags = SA_RESTART | SA_SIGINFO;
//   sigemptyset(&act.sa_mask);
//   if (sigaction(SIGSEGV, &act, &old_act)) {
//     perror("Failed to install segv handler");
//     return -1;
//   }
//   return 0;
// }

__attribute__((constructor)) void libmcmini_event_late_init() {
  if (!dmtcp_is_enabled()) {
    return;
  }

  // Initialization should NOT happen in any of the
  // `pthread*` wrapper functions, as they may be called
  // in unexpected ways prior to the `DMTCP_EVENT_INIT`.
  //
  // For example, `libatomic.so` on aarch64 calls `pthread_mutex_lock`
  // as part of its implementation (silly but so it is). This calls
  // `libmcmini.so`'s `pthread_mutex_lock` since McMini comes first.
  // If we called `libmcmini_init()` then, it's possible to end back
  // up in DMTCP (via `dlopen(3)` which DMTCP intercepts) and subsequently
  // call `pthread_mutex_lock` through `libatomic.so` _all by the same thread_.
  // Since we use `pthread_once()`, this causes a deadlock.
  //
  // Hence, we initialization ONLY NOW, and there is not danger that the
  // wrapper functions will unexpectedly recurse on themselves.
  libmcmini_init();

  // We also initialize the semaphore used by the wrapper functions
  // AFTER DMTCP restart. This ensures that the semaphore is properly
  // initialized at restart time.
  libpthread_sem_init(&dmtcp_restart_sem, 0, 0);

  // We would prefer to create the template thread
  // only during restart. However, the restart event
  // is handled by the checkpoint thread. The DMTCP
  // checkpoint thread does not expect to create an
  // extra user thread as a child of the checkpoint
  // thread. So we create the template thread here.
  //
  // The problem with `pthread_create` is that both
  // DMTCP, libpthread.so, and libmcmini.so define it.
  // We don't want to use `libmcmini's` `pthread_create`,
  // because the template thread is a `mcmini` helper thread
  // and not a user thread. We also don't want to use
  // `libpthread.so` directly, or else DMTCP won't
  // know about the thread. So we're left with DMTCP's
  // `pthread_create`.
  //
  // However, we _cannot_ create the template thread
  // upon receiving the `DMTCP_EVENT_INIT` because we need
  // to ensure that any DMTCP resources used by DMTCP's own
  // internal plugins are initialized before using the
  // DMTCP resources. DMTCP's `pthread_create` essentially
  // assumes that the DMTCP resources are already initialized.
  pthread_t template_thread_id;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  libpthread_sem_init(&template_thread_sem, 0, 0);
  libdmtcp_pthread_create(&template_thread_id, &attr, &template_thread, NULL);
  pthread_attr_destroy(&attr);
}

static void presuspend_eventHook(DmtcpEvent_t event, DmtcpEventData_t *data) {
  switch (event) {
    case DMTCP_EVENT_INIT: {
      // By default, `libmcmini_mode` is set to `PRE_DMTCP_INIT`
      // to indicate that DMTCP has not yet sent the
      // DMTCP_EVENT_INIT to `libmcmini.so`. This ensures that
      // wrapper functions simply forward their calls to the
      // next appropriate function in line. In most cases, this means
      // calling the equivalent function in `libphread.so`.
      //
      // The checkpoint thread will be created immediately after
      // DMTCP sends the `DMTCP_EVENT_INIT` but it has _not_ yet been
      // created. This means that is it not safe to move into `RECORD` mode
      // yet: we have to wait until the checkpoint thread has been created
      // before moving into `RECORD` mode.
      //
      // Why? If we set
      set_current_mode(PRE_CHECKPOINT_THREAD);
      log_verbose("DMTCP_EVENT_INIT");
      break;
    }
    case DMTCP_EVENT_PRESUSPEND:
      log_verbose("DMTCP_EVENT_PRESUSPEND");
      break;
    case DMTCP_EVENT_PRECHECKPOINT: {
      set_current_mode(PRE_CHECKPOINT);
      log_verbose("DMTCP_EVENT_PRECHECKPOINT");
      break;
    }
    case DMTCP_EVENT_RESUME:
      log_verbose("DMTCP_EVENT_RESUME");
      break;
    case DMTCP_EVENT_RESTART: {
      log_verbose("DMTCP_EVENT_RESTART callback");
      if (getenv("MCMINI_TEMPLATE_LOOP")) {
        set_current_mode(DMTCP_RESTART_INTO_TEMPLATE);
        log_debug("`MCMINI_TEMPLATE_LOOP` was set at restart-time\n");
      } else {
        set_current_mode(DMTCP_RESTART_INTO_BRANCH);
        log_debug("`MCMINI_TEMPLATE_LOOP` was not set at restart-time\n");
      }

      // During record mode, the shared memory
      // used by the `mcmini` process to control
      // the userspace threads in this process
      // is not allocated. At restart time,
      // the userspace threads may be in the middle
      // of executing wrapper functions. Once they
      // notice that the `DMTCP_EVENT_RESTART` event has
      // been sent, they will want to access this
      // region. Hence, we need to allocate it prior
      // to returning from the checkpoint thread
      //
      // NOTE: `mcmini` ensures that the shared memory region
      // is properly _initialized_, just as with classic
      // model checking.
      char shm_name[100];
      snprintf(shm_name, sizeof(shm_name), "/mcmini-%s-%lu", getenv("USER"), (long)dmtcp_virtual_to_real_pid(getppid()));
      shm_name[sizeof(shm_name) - 1] = '\0';
      mc_allocate_shared_memory_region(shm_name);

      // A target program may install a signal handler for signals such
      // as SIGSEGV. A common paradigm with such a signal handler is
      // to block forever to allow a user to attach a debugger to
      // the process for debugging. However, McMini relies on receiving
      // the SIGCHLD signal from processes which contain a segfault.
      // Therefore, on restart, we disable any handlers for SIGSEGV
      struct sigaction action;
      action.sa_handler = SIG_DFL;
      sigemptyset(&action.sa_mask);
      sigaction(SIGSEGV, &action, NULL);

      // Moreover, to ensure that the template thread is solely
      // responsible for handling SIGCHLD, by default we block
      // SIGCHLD. Blocking only in the checkpoint thread and unblocking
      // in the template thread works because the man page for
      //`pthread_sigmask(3)` reads:
      //
      // """
      // A new thread inherits a copy of its creator's signal mask.
      // """
      sigset_t sigchld;
      sigemptyset(&sigchld);
      sigaddset(&sigchld, SIGCHLD);
      pthread_sigmask(SIG_BLOCK, &sigchld, NULL);

      // NOTE: The template thread has been sleeping
      // on `template_thread_sem` during the entire
      // record phase. Only at restart time do we
      // actually wake it up to take control of the
      // other userspace threads to prepare for model checking.
      libpthread_sem_post(&template_thread_sem);
      break;
    }
    default:
      break;
  }
}

DmtcpPluginDescriptor_t presuspend_plugin = {
    DMTCP_PLUGIN_API_VERSION, DMTCP_PACKAGE_VERSION,
    "McMini DMTCP Plugin",    "McMini",
    "dmtcp@ccs.neu.edu",      "McMini deep debugging plugin",
    presuspend_eventHook};

DMTCP_DECL_PLUGIN(presuspend_plugin);
