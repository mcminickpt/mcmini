
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

#include "dmtcp/include/dmtcp.h"
#include "dmtcp/include/config.h.in"
#include "dmtcp/include/kvdb.h"
#include "dmtcp/include/util.h"
#include "dmtcp/jalib/jassert.h"
#include "dmtcp/jalib/jfilesystem.h"
#include "dmtcp/include/protectedfds.h"
#include "dmtcp/include/procselfmaps.h"
#include "mcmini_dmtcp_plugin.h"

using namespace dmtcp;

void record_before_ckpt()
{
    setenv("MCMINI_RECORD", "1", 1);
 
}

static void
mc_plugin_event_hook(DmtcpEvent_t event, DmtcpEventData_t *data)
{
    switch (event) {
    case DMTCP_EVENT_PRECHECKPOINT:
    }
}