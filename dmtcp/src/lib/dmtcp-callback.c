#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dmtcp.h"
#include "mcmini/common/exit.h"
#include "mcmini/spy/checkpointing/record.h"
#include "mcmini/spy/checkpointing/rec_list.h"
#include "mcmini/spy/intercept/interception.h"

static atomic_bool is_template_thread_alive = 0;

static void *template_thread(void *unused) {
  // Determine how many times
  int thread_count = 0;
  struct dirent *entry;
  DIR *dp = opendir("/proc/self/tasks");
  if (dp == NULL) {
    perror("opendir");
    exit(EXIT_FAILURE);
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
    sem_wait(&dmtcp_restart_sem);
  }

  printf("The template thread is finished... restarting...\n");
  int fd = open("/tmp/mcmini-fifo", O_WRONLY);
  if (fd == -1) {
    perror("open");
  }
  for (rec_list *entry = head_record_mode; entry != NULL; entry = entry->next) {
    printf("Writing entry %p (state %d)\n", entry->vo.location,
           entry->vo.mut_state);
    write(fd, &entry->vo, sizeof(visible_object));
  }
  write(fd, &empty_visible_obj, sizeof(empty_visible_obj));
  printf("The template thread has completed: looping...\n");
  fsync(fd);
  fsync(0);

  // TODO: Exit for now --> loop eventually and do multithreaded forks
  mc_exit(0);
  return NULL;
}

static void presuspend_eventHook(DmtcpEvent_t event, DmtcpEventData_t *data) {
  switch (event) {
    case DMTCP_EVENT_INIT: {
      // By default, `libmcmini_mode` is set to `PRE_DMTCP`
      // to indicate that DMTCP has not yet sent the
      // DMTCP_EVENT_INIT to `libmcmini.so`. This ensures that
      // wrapper functions simply forward their calls to the
      // next (in the sense of RTLD_NEXT) function in line
      // (most likely those in `libphread.so`) while DMTCP
      // initializes itself (i.e. the primitives manipulated
      // by DMTCP prior to restart are not recorded as part of
      // the state of the process)
      atomic_store(&libmcmini_mode, RECORD);

      // We also initialize the semaphore used by the wrapper functions
      // AFTER DMTCP restart. This ensures that the semaphore is properly
      // initialized at restart time.
      sem_init(&dmtcp_restart_sem, 0, 0);

      head_record_mode = NULL;
      printf("DMTCP_EVENT_INIT\n");
      break;
    }
    case DMTCP_EVENT_PRESUSPEND:
      printf("DMTCP_EVENT_PRESUSPEND\n");
      break;

    case DMTCP_EVENT_PRECHECKPOINT:
      atomic_store(&libmcmini_mode, PRE_CHECKPOINT);
      printf("DMTCP_EVENT_PRECHECKPOINT\n");
      break;

    case DMTCP_EVENT_RESUME:
      printf("DMTCP_EVENT_RESUME\n");
      break;

    case DMTCP_EVENT_RESTART: {
      if (!atomic_load(&is_template_thread_alive)) {
        atomic_store(&libmcmini_mode, DMTCP_RESTART);
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        libpthread_pthread_create(NULL, &attr, &template_thread, NULL);
        atomic_store(&is_template_thread_alive, 1);
      }
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
