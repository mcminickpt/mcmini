#pragma once

#ifdef __cplusplus
#error "This file should only be included in `libmcmini.so`"
#endif

#include "mcmini/common/exit.h"
#include "mcmini/common/shm_config.h"
#include "mcmini/lib/entry.h"
#include "mcmini/lib/sig.h"
#include "mcmini/lib/template.h"
#include "mcmini/mem.h"
#include "mcmini/plugins.h"
#include "mcmini/real_world/mailbox/runner_mailbox.h"
#include "mcmini/real_world/process/template_process.h"
#include "mcmini/spy/checkpointing/alloc.h"
#include "mcmini/spy/checkpointing/record.h"
#include "mcmini/spy/checkpointing/transitions.h"
#include "mcmini/spy/intercept/interception.h"
#include "mcmini/spy/intercept/wrappers.h"
