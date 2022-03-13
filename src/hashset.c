#include "hashset.h"

#define HASH_SET_UNUSED_ENTRY_VALUE (void*)0x01 // Anything but NULL

struct hash_set_iter {
    hash_table_iter_ref iter;
};

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
    if (!contained) {
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

MEMORY_ALLOC_DEF_DECL(hash_set_iter)

hash_set_iter_ref
hash_set_iter_create(hash_set_refc set)
{
    hash_set_iter_ref set_iter = hash_set_iter_alloc();
    if (set_iter) {
        // TODO: Add error checking when the hash_table_iter becomes NULL
        set_iter->iter = hash_table_iter_create(set->storage);
    }
    return set_iter;
}

void
hash_set_iter_destroy(hash_set_iter_ref set_iter)
{
    if (!set_iter) return;
    hash_table_iter_destroy(set_iter->iter);
    free(set_iter);
}

inline hash_set_entry
hash_set_iter_get_next(hash_set_iter_ref set_iter)
{
    hash_table_entry next = hash_table_iter_get_next(set_iter->iter);
    return (hash_set_entry) { .value = next.key };
}

inline bool
hash_set_iter_has_next(hash_set_iter_ref set_iter)
{
    return hash_table_iter_has_next(set_iter->iter);
}
