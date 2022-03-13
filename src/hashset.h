#ifndef DPOR_HASHSET_H
#define DPOR_HASHSET_H

#include "decl.h"
#include "common.h"
#include "hashtable.h"

STRUCT_DECL(hash_set);

STRUCT_DECL(hash_set_iter)
STRUCT_DECL(hash_set_entry)
struct hash_set_entry {
    void * value; /* A member of the set */
};

hash_set_ref hash_set_alloc(void);
hash_set_ref hash_set_create(hash_function, hash_equality_function);
hash_set_ref hash_set_copy(hash_set_refc);
void hash_set_destroy(hash_set_ref);

size_t hash_set_count(hash_set_ref);
bool hash_set_is_empty(hash_set_ref);
bool hash_set_contains(hash_set_ref, void*);
bool hash_set_insert(hash_set_ref, void*);
bool hash_set_remove(hash_set_ref, void*);

hash_set_iter_ref hash_set_iter_alloc(void);
hash_set_iter_ref hash_set_iter_create(hash_set_refc);
void hash_set_iter_destroy(hash_set_iter_ref);
hash_set_entry hash_set_iter_get_next(hash_set_iter_ref);
bool hash_set_iter_has_next(hash_set_iter_ref);

#endif //DPOR_HASHSET_H
