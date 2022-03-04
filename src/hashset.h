#ifndef DPOR_HASHSET_H
#define DPOR_HASHSET_H

#include "decl.h"
#include "common.h"
#include "hashtable.h"

STRUCT_DECL(hash_set);

hash_set_ref hash_set_alloc(void);
hash_set_ref hash_set_create(hash_function);
hash_set_ref hash_set_copy(hash_set);
void hash_set_destroy(hash_set_ref);

size_t hash_set_count(hash_set_ref);
bool hash_set_is_empty(hash_set_ref);
bool hash_set_contains(hash_set_ref, void*);
bool hash_set_insert(hash_set_ref, void*);
bool hash_set_remove(hash_set_ref, void*);

#endif //DPOR_HASHSET_H
