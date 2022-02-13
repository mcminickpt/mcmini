#ifndef DPOR_ARRAY_H
#define DPOR_ARRAY_H

#include <errno.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "decl.h"

STRUCT_DECL(array);

/*
 * ---- Memory Interface ----
 */

extern array_ref array_create(void);
extern array_ref array_clone(void**, size_t);
extern void array_destroy(array_ref _Nonnull, void(* _Nullable free_each)(void * _Nullable));
extern void array_destroy_opaque(void*);

/*
 * ---- Operations ----
 */

/*
 * Array count
 */
extern uint32_t array_count(array_refc _Nonnull);
extern bool array_is_empty(array_refc _Nonnull);

/*
 * Array elements and manipulations
 */
extern void *array_get(array_refc _Nonnull, int index);
extern void *array_get_last(array_refc _Nonnull);
extern void *array_get_first(array_refc _Nonnull);
extern void array_set(array_ref _Nonnull, int index, const void **data);
extern void array_swap(array_ref _Nonnull, int i1, int i2);

/*
 * Adding/Removing elements
 */
extern void array_append(array_ref _Nonnull, const void *data);
extern void array_append_array(array_ref _Nonnull, const struct array*);
extern void array_insert(array_ref _Nonnull, int index, const void *data);
extern void *array_remove(array_ref _Nonnull, int index);
extern void *array_remove_first(array_ref _Nonnull);
extern void *array_remove_last(array_ref _Nonnull);

/*
 * Copying contents
 */
extern array_ref array_shallow_cpy(array_refc _Nonnull);
extern array_ref array_deep_cpy(array_refc _Nonnull, void*(* _Nonnull cpy)(void * _Nullable));

/*
 * Large-scale array manipulations
 */
extern array_ref array_filter(const struct array*, bool(* _Nonnull filter)(void * _Nullable));
extern array_ref array_map(array_refc _Nonnull, void*(* _Nonnull map)(void * _Nullable));
extern void *array_reduce(array_refc _Nonnull, void*(* _Nonnull reduce)(void * _Nullable, void * _Nullable));
extern void array_for_each(array_refc _Nonnull, void(* _Nonnull each)(void * _Nullable));

#endif //DPOR_ARRAY_H
