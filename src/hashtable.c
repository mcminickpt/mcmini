#include "hashtable.h"

STRUCT_DECL(hash_table_entry)

struct hash_table_iter {
    uint64_t ent;                            /* The index of the current valid entry in the hash table */
    uint64_t last_valid;                     /* Points to the last valid index in the hash_table */
    uint64_t htable_size;                    /* Cached number of elements of the hash_table that is iterated through */
    uint64_t htable_num_slots_to_search;     /* Cached number of slots for elements in the hash_table that is iterated through */
    hash_table_refc iterated;                /* The hash table that is iterated upon */
};

/* A hash table entry that represents the absence of an entry */
/*
 * NOTE: The fields must all represent the zero value. This allows rehashing the table to happen more
 * quickly since a call to bzero empties the hash table
 */
static const hash_table_entry hash_table_invalid_entry = { .valid = false, .key = NULL, .value = NULL };

struct hash_table {
    size_t count;                       /* The number of occupied entries in the hash table */
    size_t len;                         /* The length, in bytes, of the hash table */
    hash_table_entry *base;             /* The base address of the hash table's contents */
    hash_function hasher;               /* A function to apply automatically to */
    hash_equality_function equals;      /* Used to resolve hash collisions -> determines when we should */
};

MEMORY_ALLOC_DEF_DECL(hash_table)

hash_table_ref
hash_table_create(hash_function hash_function, hash_equality_function equals) {
    if (!equals || !hash_function) return NULL;

    hash_table_ref ref = (hash_table_ref)malloc(sizeof(hash_table));
    if (ref) {
        ref->count = 0;
        ref->len = 0;
        ref->base = NULL;
        ref->equals = equals;
        ref->hasher = hash_function;
    }
    return ref;
}

hash_table_ref
hash_table_copy(hash_table_refc other)
{
    if (!other) return NULL;

    hash_table_ref ref = hash_table_alloc();
    if (ref) {
        ref->count = other->count;
        ref->len = other->len;
        ref->hasher = other->hasher;
        ref->equals = other->equals;

        if (other->len > 0) {
            void *base = malloc(other->len);
            ref->base = memcpy(base, other->base, other->len);
        } else {
            ref->base = NULL;
        }
    }

    return ref;
}

void
hash_table_destroy(hash_table_ref ref) {
    if (!ref) {
        errno = EINVAL;
        return;
    }

    if (ref->base) {
        free(ref->base);
    }
    free(ref);
}

/**
 * A hash function that computes a hash value from
 * an unsigned 64-bit integer
 *
 * @param key a value to be hashed
 * @return a 64-bit hash value of the key
 */
static uint64_t
hash_key(uint64_t key) {
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = ((key >> 16) ^ key) * 0x45d9f3b;
    key = (key >> 16) ^ key;
    return key;
}

/**
 * Computes the number of possible entries that
 * could be in the hash table at its given count
 *
 * @param ref a reference to a hash table
 * @return the number of possible entries in the hash
 * table, or -1 if the hash table is -1
 */
static uint64_t
hash_table_num_slots(hash_table_refc ref) {
    return ref ? ref->len / sizeof(hash_table_entry) : UINT64_MAX;
}

/**
 * Computes the index into the given hash table's
 * dynamic storage that corresponds to the given key
 *
 * Every key corresponds to a unique index in the underlying
 * array storage of the hash table. However, since it is possible
 * for hash collisions to occur, the hashed index that corresponds
 * for any given key is *not* necessarily the same as the index
 * in the table where the data associated with the given key
 * is stored (see the `hash_table_probe()` function for more details)
 *
 * @param key a key that needs to be mapped to an index into
 * the hash table's current storage
 *
 * @return an index into the hash table that the given
 * key maps to
 */
static uint64_t
hash_table_map_key(hash_table_refc ref, void *key)
{
    uint64_t hash_value = hash_key(ref->hasher(key));
    uint64_t num_entries = hash_table_num_slots(ref);
    return hash_value % num_entries;
}

static void
hash_table_rehash(hash_table_ref ref, hash_table_entry *old_base)
{
    if (!ref || !ref->base) {
        errno = EINVAL;
        return;
    }

    // We just doubled the number of slots -> the old_base had half of the new one
    uint64_t entries = hash_table_num_slots(ref) / 2;
    for (int i = 0; i < entries; i++) {
        hash_table_entry_ref ent = &old_base[i];

        if (ent->valid) {
            uint64_t dest = hash_table_map_key(ref, ent->key);
            ref->base[dest] = *ent;
        }
    }
}

static void
hash_table_unforced_grow(hash_table_ref ref)
{
    if (ref->base == NULL) {
        ref->base = (hash_table_entry*) malloc(sizeof(hash_table_entry));
        ref->base[0] = hash_table_invalid_entry;
        ref->len = sizeof(hash_table_entry);
    }

    // Compute the number of bytes occupied by the entries in the table
    size_t fill = ref->count * sizeof(hash_table_entry);
    double filld = (double)fill;

    if (filld >= ((double)ref->len * REHASH_FACTOR)) {
        size_t old_len = ref->len;
        size_t new_len = 2 * old_len;

        // Prepare to rehash into the new memory region
        hash_table_entry *old_base = ref->base;
        hash_table_entry *new_base = (hash_table_entry*)malloc(new_len);
        explicit_bzero(new_base, new_len); /* All entries are invalid */

        // Rehash the first `rehash_size` entries in the table
        ref->len *= 2;
        ref->base = new_base;
        hash_table_rehash(ref, old_base);

        // Free the old memory
        free(old_base);
    }
}

static uint64_t
hash_table_probe_get(hash_table_refc ref, void *key) {
    uint64_t best_match = hash_table_map_key(ref, key);
    if (ref->count == 0)
        return best_match;

    uint64_t num_ents = hash_table_num_slots(ref);
    uint64_t num_searched = 0ul;
    hash_table_entry cur;

    while ((cur = ref->base[best_match]).valid && ++num_searched <= num_ents) {
        if (ref->equals(key, cur.key))
            return best_match;
        best_match = (best_match + 1) % num_ents;
    }

    return UINT64_MAX;
}

static uint64_t
hash_table_probe_set(hash_table_ref ref, void *key, bool *replace) {
    uint64_t best_match = hash_table_map_key(ref, key);
    if (ref->count == 0)
        return best_match;

    uint64_t num_ents = hash_table_num_slots(ref);
    hash_table_entry cur;

    while ((cur = ref->base[best_match]).valid) {
        if (ref->equals(cur.key, key)) {
            *replace = true;
            return best_match;
        }
        best_match = (best_match + 1) % num_ents;
    }

    *replace = false;
    return best_match;
}

size_t
hash_table_count(hash_table_refc ref) {
    if (!ref) {
        errno = EINVAL;
        return 0;
    }

    return ref->count;
}

bool
hash_table_is_empty(hash_table_refc ref)
{
    if (!ref) return true;
    return ref->count == 0;
}

void*
hash_table_get(hash_table_refc ref, void *key) {
    if (!ref || !ref->base) {
        errno = EINVAL;
        return NULL;
    }

    uint64_t dest;
    if ((dest = hash_table_probe_get(ref, key)) == UINT64_MAX)
        return NULL;

    return ref->base[dest].value;
}

void
hash_table_set(hash_table_ref ref, void * key, void *value) {
    if (!ref) {
        errno = EINVAL;
        return;
    }
    hash_table_unforced_grow(ref); /* Possibly grow the hash table */

    bool replace = false;
    uint64_t dest = hash_table_probe_set(ref, key, &replace);

    ref->base[dest] = (hash_table_entry){ .valid = true, .key = key, .value = value };
    replace ? (void)0 : ref->count++;
}

void*
hash_table_remove(hash_table_ref ref, void *key) {
    if (!ref || !ref->base) {
        errno = EINVAL;
        return NULL;
    }

    const uint64_t index_to_remove = hash_table_map_key(ref, key);
    const uint64_t num_ents = hash_table_num_slots(ref);

    ref->base[index_to_remove] = hash_table_invalid_entry;
    ref->count--;

    // With linear probing, it is not sufficient
    // to merely mark the slot as empty. You must also perform
    // more work to ensure that other hash-collided
    // values still are correctly found. See
    // https://en.wikipedia.org/wiki/Linear_probing for details
    hash_table_entry *cur = NULL;

    uint64_t index = (index_to_remove + 1) % num_ents;
    uint64_t index_to_replace = index_to_remove;
    while ((cur = &ref->base[index])->valid) {

        // Asks the question: does the entry at this *index*
        // *want* to be located before the spot we are replacing
        uint64_t index_for_key_at_cur = hash_table_map_key(ref, cur->key);
        if (index_for_key_at_cur <= index_to_replace) {
            ref->base[index_to_remove] = *cur;
            ref->base[index] = hash_table_invalid_entry;
            index_to_replace = index;
        }
        index = (index + 1) % num_ents;
    }
}

void
hash_table_clear(hash_table_ref ref)
{
    if (!ref) return;
    explicit_bzero(ref->base, ref->len);
    ref->count = 0;
}

MEMORY_ALLOC_DEF_DECL(hash_table_iter);

hash_table_iter_ref
hash_table_iter_create(hash_table_refc table)
{
    hash_table_iter_ref ref = hash_table_iter_alloc();
    if (ref) {
        ref->ent = 0;
        ref->last_valid = 0;
        ref->iterated = table;
        ref->htable_size = hash_table_count(table);
    }
    return ref;
}

void
hash_table_iter_destroy(hash_table_iter_ref ref)
{
    if (!ref) return;
    free(ref);
}

hash_table_entry
hash_table_iter_get_next(hash_table_iter_ref iter)
{
    if (!iter) return hash_table_invalid_entry;

    uint64_t start = iter->last_valid;

    for (uint64_t i = start; i < iter->htable_num_slots_to_search; i++) {
        hash_table_entry_ref ent = &iter->iterated->base[i];
        if (ent->valid) {
            iter->last_valid = i;
            iter->ent++;
            return *ent;
        }
    }
    return hash_table_invalid_entry;
}

inline bool
hash_table_iter_has_next(hash_table_iter_ref iter)
{
    if (!iter) return false;
    return iter->ent < iter->htable_size;
}

