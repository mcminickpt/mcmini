#include "hashset.h"

#define HASH_SET_UNUSED_ENTRY_VALUE (void*)0x01 // Anything but NULL

struct hash_set {
    hash_table_ref storage; /* Backs the set */
};

MEMORY_ALLOC_DEF_DECL(hash_set);

hash_set_ref
hash_set_create(hash_function hfunc, hash_equality_function equals)
{
    hash_set_ref ref = hash_set_alloc();
    if (!ref) return NULL;
    ref->storage = hash_table_create(hfunc, equals);
    return ref;
}

hash_set_ref
hash_set_copy(hash_set_refc other)
{
    if (!other) return NULL;
    hash_set_ref hset = hash_set_alloc();
    if (!hset) return NULL;
    hset->storage = hash_table_copy(other->storage);
    return hset;
}

void
hash_set_destroy(hash_set_ref ref)
{
    if (!ref) return;
    hash_table_destroy(ref->storage);
    free(ref);
}

bool
hash_set_contains(hash_set_ref ref, void *value)
{
    if (!ref) return false;
    return hash_table_get(ref->storage, value) != NULL;
}

size_t
hash_set_count(hash_set_ref ref)
{
    if (!ref) return 0;
    return hash_table_count(ref->storage);
}

bool
hash_set_is_empty(hash_set_ref ref)
{
    if (!ref) return true;
    return hash_table_is_empty(ref->storage);
}

bool
hash_set_insert(hash_set_ref ref, void *value)
{
    if (!ref) return false;

    bool contained = hash_set_contains(ref, value);
    if (contained) {
        hash_table_set(ref->storage, value, HASH_SET_UNUSED_ENTRY_VALUE);
        return true;
    }
    return false;
}

bool
hash_set_remove(hash_set_ref ref, void *value)
{
    if (!ref) return false;

    bool contained = hash_set_contains(ref, value);
    if (contained) {
        hash_table_remove(ref->storage, value);
        return true;
    }
    return false;
}
