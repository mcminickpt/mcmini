#ifndef DPOR_ARRAY_H
#define DPOR_ARRAY_H

#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include "decl.h"

STRUCT_DECL(array);

/*
 * ---- Memory Interface ----
 */
array_ref array_create(void);
array_ref array_clone(void**, size_t);
void array_destroy(array_ref, void(* free_each)(void *));
void array_destroy_opaque(void*);

/*
 * ---- Operations ----
 */

/*
 * Array count
 */
uint32_t array_count(array_refc);
bool array_is_empty(array_refc);

/*
 * Array elements and manipulations
 */
void *array_get(array_refc, uint32_t index);
void *array_get_last(array_refc);
void *array_get_first(array_refc);
void array_set(array_ref, uint32_t index, const void **data);
void array_swap(array_ref, uint32_t i1, uint32_t i2);

/*
 * Adding/Removing elements
 */
void array_append(array_ref, const void *data);
void array_append_array(array_ref, const struct array*);
void array_insert(array_ref, uint32_t index, const void *data);
void *array_remove(array_ref, uint32_t index);
void *array_remove_first(array_ref);
void *array_remove_last(array_ref);

/*
 * Copying contents
 */
array_ref array_shallow_cpy(array_refc);
array_ref array_deep_cpy(array_refc, void*(*  cpy)(void * ));

/*
 * Large-scale array manipulations
 */
array_ref array_filter(array_refc, bool(* filter)(void *));
array_ref array_map(array_refc, void*(*map)(void *));
void *array_reduce(array_refc, void*(*reduce)(void *, void *));
void array_for_each(array_refc, void(*each)(void *));

#endif //DPOR_ARRAY_H
