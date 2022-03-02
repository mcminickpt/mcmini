#ifndef DPOR_HASHTABLE_H
#define DPOR_HASHTABLE_H

#include <stdlib.h>
#include <stdint.h>
#include "array.h"
#include "decl.h"

typedef uint64_t hash_t;
typedef hash_t(*hash_function)(void*);
#define REHASH_FACTOR (0.5)

STRUCT_DECL(hash_table);
MEMORY_API_DECL(hash_table);

/*
 * Hash table properties
 */
size_t hash_table_count(hash_table_ref);
void hash_table_set_hash_function(hash_table_ref, hash_function);

/*
 * Adding an element
 */
void *hash_table_get(hash_table_ref, hash_t key);
void *hash_table_get_implicit(hash_table_ref, void *key);
void hash_table_set(hash_table_ref, hash_t key, void *value);
void hash_table_set_implicit(hash_table_ref, void *key, void *value); /* Uses the implicit hash function provided to the table */
void *hash_table_remove(hash_table_ref, hash_t key);
void *hash_table_remove_implicit(hash_table_ref, void* value);

#endif //DPOR_HASHTABLE_H
