#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dmtcp.h"
#include "mcmini/mcmini.h"

// We probably won't need the '#undef', but just in case a .h file defined it:
#undef dmtcp_mcmini_is_loaded
int dmtcp_mcmini_is_loaded() { return 1; }

static sem_t template_thread_sem;
static pthread_cond_t template_thread_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t template_thread_mut = PTHREAD_MUTEX_INITIALIZER;

void thread_handle_after_dmtcp_restart(void) {
  printf("thread_handle_after_dmtcp_restart\n");
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
      pthread_mutex_lock(&template_thread_mut);
      while (get_current_mode() != TARGET_BRANCH_AFTER_RESTART)
        pthread_cond_wait(&template_thread_cond, &template_thread_mut);
      pthread_mutex_unlock(&template_thread_mut);
    }
    case DMTCP_RESTART_INTO_BRANCH: {
      // In the case of calling `dmtcp_restart` fpr each branch, we
      // are expected to immediately talk to the model checker.
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

static void *template_thread(void *unused) {
  // Phase 1. The template thread is created at record-time
  // _prior_ to checkpoint. It must not intervene until exclusively
  // after a `dmtcp_restart`.
  //
  // This is accomplished by waiting on a semaphore that is only incremented
  // once the `DMTCP_EVENT_RESTART` is sent to `libmcmini.so`.
  libpthread_sem_wait(&template_thread_sem);

  // Phase 2. Wait for all userspace threads to move into a stable state
  //
  // After the `DMTCP_EVENT_RESTART` event is send to `libmcmini.so`, all
  // userspace threads will resume execution. These userspace threads ,
  //
  // TODO: Even in the event in which we don't need to transfer the records
  // during the RECORD phase, we still wait for the userspace threads to go into
  // the "stable" state. This is probably not needed. INdeed, for the
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

  printf(
      "There are %d threads... waiting for them to get into a consistent "
      "state...\n",
      thread_count);
  for (int i = 0; i < thread_count; i++) {
    libpthread_sem_wait(&dmtcp_restart_sem);
  }
  printf("The threads are now in a consistent state: %s\n",
         getenv("MCMINI_NEEDS_STATE"));

  // Phase 3. Once in a stable state, check if `mcmini` needs to construct
  // a model of what we've recorded.
  if (getenv("MCMINI_NEEDS_STATE")) {
    printf("The template thread is finished... restarting...\n");

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
        printf("Writing mutex entry %p (state %d)\n", entry->vo.location,
               entry->vo.mut_state);
      } else if (entry->vo.type == THREAD) {
        printf("Writing thread entry %p (id %d, status: %d)\n",
               (void *)entry->vo.thrd_state.pthread_desc,
               entry->vo.thrd_state.id, entry->vo.thrd_state.status);
      } else {
        libc_abort();
      }

      int sz = write(fd, &entry->vo, sizeof(visible_object));
      assert(sz == sizeof(visible_object));
    }
    int sz = write(fd, &empty_visible_obj, sizeof(empty_visible_obj));
    assert(sz == sizeof(visible_object));
    printf("The template thread has completed: exiting...\n");
    fsync(fd);
    fsync(0);
    close(fd);
  }

  volatile struct mcmini_shm_file *shm_file = global_shm_start;
  volatile struct template_process_t *tpt = &shm_file->tpt;

  switch (get_current_mode()) {
    case DMTCP_RESTART_INTO_BRANCH: {
      printf("DMTCP_RESTART_INTO_BRANCH\n");

      pid_t target_branch_pid = dmtcp_virtual_to_real_pid(getpid());
      tpt->cpid = target_branch_pid;
      sem_post((sem_t *)&tpt->mcmini_process_sem);
      atexit(&mc_exit_main_thread_in_child);
      break;
    }
    case DMTCP_RESTART_INTO_TEMPLATE: {
      printf("DMTCP_RESTART_INTO_TEMPLATE\n");
      mc_template_process_loop_forever(&multithreaded_fork);

      // Reaching this point means that we're in the branch: the
      // parent process (aka the template) will never exit
      // the above call to `mc_template_process_loop_forever()`.
      set_current_mode(TARGET_BRANCH_AFTER_RESTART);

      // Recall that the userspace threads in the template process
      // were idling/doing nothing. Indeed, those threads exist ONLY
      // to ensure that `multithreaded_fork()` clones them.
      //
      // Now that we're finally in the branch, we can
      pthread_mutex_lock(&template_thread_mut);
      pthread_cond_broadcast(&template_thread_cond);
      pthread_mutex_unlock(&template_thread_mut);
    }
    default:
      break;
  }

  // Exiting from the template thread is fine:
  // once we're in the target branch, we no longer care
  // about it anyways.
  //
  // NOTE: This is true for both repeated `dmtcp_restart` AND for multithreaded
  // forking.
  return NULL;
}

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
      printf("DMTCP_EVENT_INIT\n");
      break;
    }
    case DMTCP_EVENT_PRESUSPEND:
      printf("DMTCP_EVENT_PRESUSPEND\n");
      break;
    case DMTCP_EVENT_PRECHECKPOINT: {
      set_current_mode(PRE_CHECKPOINT);
      printf("DMTCP_EVENT_PRECHECKPOINT\n");
      break;
    }
    case DMTCP_EVENT_RESUME:
      printf("DMTCP_EVENT_RESUME\n");
      break;
    case DMTCP_EVENT_RESTART: {
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

      if (getenv("MCMINI_TEMPLATE_LOOP")) {
        set_current_mode(DMTCP_RESTART_INTO_TEMPLATE);
      } else {
        set_current_mode(DMTCP_RESTART_INTO_BRANCH);
      }

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
