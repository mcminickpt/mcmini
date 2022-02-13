#ifndef DPOR_HASHTABLE_H
#define DPOR_HASHTABLE_H

#include <stdlib.h>
#include <stdint.h>
#include "array.h"
#include "decl.h"

#define REHASH_FACTOR (0.5)
STRUCT_DECL(hash_table);
MEMORY_API_DECL(hash_table);


/*
 * Hash table properties
 */
extern int hash_table_size(hash_table_ref);

/*
 * Adding an element
 */
extern void *hash_table_get(hash_table_ref, uint64_t key);
extern void hash_table_set(hash_table_ref, uint64_t key, void *value);
extern void *hash_table_remove(hash_table_ref, uint64_t key);

#endif //DPOR_HASHTABLE_H
