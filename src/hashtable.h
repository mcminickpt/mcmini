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

STRUCT_DECL(hash_table_iter)
STRUCT_DECL(hash_table_entry)
struct hash_table_entry {
    bool valid;                /* Whether or not this hash entry is valid (only defined within bounds) */
    uint64_t key;              /* The identifier that is mapped by the hash function */
    void * value;              /* The value associated with the paired key */
};

size_t hash_table_count(hash_table_refc);
bool hash_table_is_empty(hash_table_refc);
void hash_table_set_hash_function(hash_table_ref, hash_function);

void *hash_table_get(hash_table_refc, hash_t key);
void *hash_table_get_implicit(hash_table_refc, void *key);
void hash_table_set(hash_table_ref, hash_t key, void *value);
void hash_table_set_implicit(hash_table_ref, void *key, void *value); /* Uses the implicit hash function provided to the table */
void *hash_table_remove(hash_table_ref, hash_t key);
void *hash_table_remove_implicit(hash_table_ref, void* value);
void hash_table_clear(hash_table_ref);

hash_table_iter_ref hash_table_iter_alloc(void);
hash_table_iter_ref hash_table_iter_create(hash_table_refc);
void hash_table_iter_destroy(hash_table_iter_ref);
hash_table_entry hash_table_iter_get_next(hash_table_iter_ref);
bool hash_table_iter_has_next(hash_table_iter_ref);



#endif //DPOR_HASHTABLE_H
