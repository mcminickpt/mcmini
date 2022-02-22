#ifndef DPOR_HASHTABLE_H
#define DPOR_HASHTABLE_H

#include <stdlib.h>
#include <stdint.h>
#include "array.h"
#include "decl.h"

typedef uint64_t hash_t;
#define REHASH_FACTOR (0.5)

STRUCT_DECL(hash_table);
MEMORY_API_DECL(hash_table);

/*
 * Hash table properties
 */
int hash_table_size(hash_table_ref);

/*
 * Adding an element
 */
void *hash_table_get(hash_table_ref, hash_t key);
void hash_table_set(hash_table_ref, hash_t key, void *value);
void *hash_table_remove(hash_table_ref, hash_t key);

#endif //DPOR_HASHTABLE_H
