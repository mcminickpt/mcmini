#include "hashset.h"

#define HASH_SET_UNUSED_ENTRY_VALUE (void*)0x01 // Anything but NULL

struct hash_set {
    hash_table_ref storage; /* Backs the set */
};

MEMORY_ALLOC_DEF_DECL(hash_set);

hash_set_ref
hash_set_create(hash_function hfunc)
{
    hash_set_ref ref = hash_set_alloc();
    if (!ref) return NULL;

    ref->storage = hash_table_create();
    hash_table_set_hash_function(ref->storage, hfunc);

    return ref;
}

hash_set_ref
hash_set_copy(hash_function)
{

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
    return hash_table_get_implicit(ref->storage, value) != NULL;
}

bool
hash_set_insert(hash_set_ref ref, void *value)
{
    if (!ref) return false;

    bool contained = hash_set_contains(ref, value);
    if (contained) {
        hash_table_set_implicit(ref->storage, value);
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
        hash_table_remove_implicit(ref->storage, value);
        return true;
    }
    return false;
}
