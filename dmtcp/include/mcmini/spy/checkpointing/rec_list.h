#pragma once

#include "mcmini/spy/checkpointing/objects.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rec_list {
  visible_object vo;
  struct rec_list *next;
} rec_list;


#ifdef __cplusplus
} // extern "C"
#endif
