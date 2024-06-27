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
      int fd = open("/tmp/mcmini-fifo", 0666);
      for (rec_list* entry = head; entry != NULL; entry = head->next) {
        write(fd, &entry->vo, sizeof(visible_object));
      }
      close(fd);

      // For now, we can simply continue execution.
      printf("DMTCP_EVENT_RESTART completed: exiting...\n");
      exit(0);
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
