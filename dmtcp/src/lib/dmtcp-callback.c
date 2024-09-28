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

static void *template_thread(void *unused) {
  libpthread_sem_wait(&template_thread_sem);

  // Determine how many times
  int thread_count = 0;
  struct dirent *entry;
  DIR *dp = opendir("/proc/self/task");
  if (dp == NULL) {
    perror("opendir");
    mc_exit(EXIT_FAILURE);
  }

  while ((entry = readdir(dp))) {
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      thread_count++;
    }
  }

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

  printf("The template thread is finished... restarting...\n");
  int fd = open("/tmp/mcmini-fifo", O_WRONLY);
  if (fd == -1) {
    perror("open");
  }
  for (rec_list *entry = head_record_mode; entry != NULL; entry = entry->next) {
    if (entry->vo.type == MUTEX) {
      printf("Writing mutex entry %p (state %d)\n", entry->vo.location,
             entry->vo.mut_state);
    } else if (entry->vo.type == THREAD) {
      printf("Writing thread entry %p (id %d, status: %d)\n",
             (void *)entry->vo.thrd_state.pthread_desc, entry->vo.thrd_state.id,
             entry->vo.thrd_state.status);
    } else {
      libc_abort();
    }

    write(fd, &entry->vo, sizeof(visible_object));
  }
  write(fd, &empty_visible_obj, sizeof(empty_visible_obj));
  printf("The template thread has completed: looping...\n");
  fsync(fd);
  fsync(0);
  int dummy = 1;
  while (dummy)
    ;
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
      // DMTCP sends the `DMTCP_EVENT_INIT`
      atomic_store(&libmcmini_mode, PRE_CHECKPOINT_THREAD);
      printf("DMTCP_EVENT_INIT\n");
      break;
    }
    case DMTCP_EVENT_PRESUSPEND:
      printf("DMTCP_EVENT_PRESUSPEND\n");
      break;
    case DMTCP_EVENT_PRECHECKPOINT: {
      atomic_store(&libmcmini_mode, PRE_CHECKPOINT);
      printf("DMTCP_EVENT_PRECHECKPOINT\n");
      break;
    }
    case DMTCP_EVENT_RESUME:
      printf("DMTCP_EVENT_RESUME\n");
      break;
    case DMTCP_EVENT_RESTART: {
      atomic_store(&libmcmini_mode, DMTCP_RESTART);

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

      // NOTE: The template thread has been sleeping
      // on `template_thread_sem` during the entire
      // record phase. Only at restart time do we
      // actually wake it up to take control of the
      // other userspace threads for model checking.
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
