#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dmtcp.h"
#include "mcmini/spy/checkpointing/record.h"

static void presuspend_eventHook(DmtcpEvent_t event, DmtcpEventData_t *data) {
  switch (event) {
    case DMTCP_EVENT_INIT:
      libmcmini_mode = RECORD;
      printf("DMTCP_EVENT_INIT\n");
      break;

    case DMTCP_EVENT_PRESUSPEND:
      printf("DMTCP_EVENT_PRESUSPEND\n");
      break;

    case DMTCP_EVENT_PRECHECKPOINT:
      libmcmini_mode = PRE_CHECKPOINT;
      printf("DMTCP_EVENT_PRECHECKPOINT\n");
      break;

    case DMTCP_EVENT_RESUME:
      printf("DMTCP_EVENT_RESUME\n");
      break;

    case DMTCP_EVENT_RESTART: {
      // TODO: Make sure the write completes fully
      printf("DMTCP_EVENT_RESTART starting...\n");
      int fd = open("/tmp/mcmini-fifo", O_WRONLY);
      if (fd == -1) {
        perror("open");
      }
      for (rec_list* entry = head_record_mode; entry != NULL; entry = entry->next) {
        printf("Writing entry %p (state %d)\n", entry->vo.location, entry->vo.mutex_state);
        write(fd, &entry->vo, sizeof(visible_object));
        add_rec_entry_restart_mode(&entry->vo);
      }
      write(fd, &empty_visible_obj, sizeof(empty_visible_obj));

      // For now, we can simply continue execution.
      printf("DMTCP_EVENT_RESTART completed: exiting...\n");

      libmcmini_mode = TARGET_BRANCH;
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
