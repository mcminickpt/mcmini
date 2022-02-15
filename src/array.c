#include "array.h"

/**
 * A dynamically-count array containing arbitrary contents
 *
 * An array is a data structure describing a collection of
 * data called elements. You can think of the elements as
 * being part of a contiguous collection, with each element
 * described in the new collection by its location in the array
 * (its index)
 *
 * Note: The array is not thread-safe. You must ensure that
 * concurrent accesses are properly synchronized
 */
struct array {
    void *base;          /* Base pointer of the array's contents */
    size_t count;        /* Number of elements in the array */
    size_t len;          /* Size in bytes of the entire array */
};

/**
 * Allocates a new empty array on the heap
 *
 * @return a reference to the new memory
 * on the heap allocated for the array, or NULL
 * if no more memory is available for allocation
 */
array_ref
array_create(void) {
    array_ref new_ref = (array_ref)malloc(sizeof(struct array));

    if (new_ref) {
        new_ref->base = NULL;
        new_ref->count = 0;
        new_ref->len = 0;
    }

    return new_ref;
}

/**
 * Clones the contents of a normal C array
 *
 * Each address copied into the new array
 * will be maintained by the new array; hence,
 * after passing the *contents* of the C array
 * to the new array struct, the lifetime of that
 * memory is (at least partially) managed by the array
 *
 * @param contents a reference to an array containing the
 * addresses whose contents
 * @param count the number of elements in the *contents* array
 * @return a new array of count `count` elements where each
 * element in the corresponds to the same element in the C-array
 */
array_ref
array_clone(void **contents, size_t count) {

    array_ref cpy;
    if ((cpy = array_create()) == NULL) {
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        array_append(cpy, contents[i]);
    }

    return cpy;
}

/**
 * Frees a previously allocated array (via a call to the
 * `array_alloc()` function)
 *
 * After you are done using an array allocated using the
 * array API, you must clean up the memory reserved for
 * the array, as well as possibly any data you allocated
 * that is contained exclusively in the array; otherwise
 * you run the risk of introducing a memory leak.
 *
 * Attempting to free an array that was not allocated with
 * `array_alloc()` is undefined behavior (expect extremely
 * strange things to happen)
 *
 * @param ref a reference to an array previously allocated
 * via a call to the `array_create()` function. If the reference
 * is NULL, the function does nothing and sets errno to EINVAL.
 * Passing a reference to an array that was not previously allocated
 * by `array_alloc`
 *
 * @param free_each a function that is invoked for each element
 * in the array. If the function pointer isn't NULL, each element
 * is freed according to the function
 */
void
array_destroy(array_ref ref, void(*free_each)(void*)) {
    if (!ref) {
        errno = EINVAL;
        return;
    }

    if (free_each) {
        array_for_each(ref, free_each);
    }

    if (ref->base) {
        free(ref->base);
    }
    free(ref);
}

void
array_destroy_opaque(void *opaque) {
    array_destroy(opaque, free);
}

/**
 * Determines the number of elements in the specified array
 *
 * @param ref a reference to an array. If the reference
 * is NULL, the function does nothing and sets errno to EINVAL
 *
 * @return the number of elements in the array, or -1 if the
 * reference provided is NULL
 */
uint32_t
array_count(array_refc ref) {
    if (!ref) {
        errno = EINVAL;
        return -1;
    }

    return ref->count;
}

static bool
array_invalid_index(array_refc ref, int index) {
    return index < 0 || index >= ref->count;
}

/**
 * Determines whether or not the given array is empty
 * (i.e. if it contains no elements)
 *
 * An array is empty if the number of elements it contains
 * is exactly 0
 *
 * @param ref a reference to an array. If the reference
 * is NULL, the function does nothing and sets errno to EINVAL
 *
 * @return a boolean specifying whether or not the array provided
 * to the function doesn't have any elements in it
 */
bool
array_is_empty(array_refc ref) {
    if (!ref) {
        errno = EINVAL;
        return false;
    }

    return ref->count == 0;
}

/**
 * Retrieves an element from an array at the specified index
 *
 * @param ref the array to retrieve an element from
 * @param index the index of the element to retrieve
 * @return the value at the specified index, or NULL either
 * if the reference is NULL or if the index is invalid
 */
void*
array_get(array_refc ref, int index) {
    if (!ref || array_invalid_index(ref, index)) {
        errno = EINVAL;
        return NULL;
    }
    return (void*)(*((uint64_t*)(ref->base) + index));
}

void*
array_get_first(array_refc ref) {
    return array_get(ref, 0);
}

void*
array_get_last(array_refc ref) {
    return array_get(ref, (int)ref->count - 1);
}

void
array_set(array_ref ref, int index, const void **data) {
    if (!ref || array_invalid_index(ref, index)) {
        errno = EINVAL;
        return;
    }

    void *old = ref->base + index * sizeof(void*);
    memcpy(old, data, sizeof(void*));
}

void
array_swap(array_ref ref, int i1, int i2) {
    if (!ref || array_invalid_index(ref, i1) || array_invalid_index(ref, i2)) {
        errno = EINVAL;
        return;
    }

    void *old1 = array_get(ref, i1);
    void *old2 = array_get(ref, i2);
    array_set(ref, i1, old2);
    array_set(ref, i2, old1);
}

static void
array_grow(array_ref ref) {
    if (!ref->base) {
        ref->base = malloc(1);
        ref->len = 1;
    }
    else {
        size_t cur = ref->len;
        ref->base = realloc(ref->base, ref->len = 2 * cur);
    }
}

void
array_append(array_ref ref, const void *data) {
    if (!ref) {
        errno = EINVAL;
        return;
    }

    size_t cur = ref->count * sizeof(void*);

    // Decide if the memory should grow.
    // The count is doubled each time to prevent
    // high overhead from memory allocations
    while (ref->len < cur + sizeof(void*))
        array_grow(ref);

    array_set(ref, ref->count++, &data);
}

void
array_append_array(array_ref ref, array_refc other) {
    if (!ref || !other) {
        errno = EINVAL;
        return;
    }

    for (int i = 0; i < array_count(other); i++)
        array_append(ref, array_get(other, i));

}

void
array_insert(array_ref ref, int index, const void *data) {
    if (!ref || array_invalid_index(ref, index)) {
        errno = EINVAL;
        return;
    }

    void *first = array_get(ref, index);
    void *last = array_get_last(ref);

    // Swap all of the other elements
    for (int i = index; i < ref->count - 1; i++) {
        array_set(ref, i + 1, array_get(ref, i));
    }

    // Put in the new data and restore the last element
    array_set(ref, index, first);
    array_append(ref, last);
}

void*
array_remove(array_ref ref, int index) {
    if (!ref || array_invalid_index(ref, index)) {
        errno = EINVAL;
        return NULL;
    }

    if (array_is_empty(ref))
        return NULL;

    // __ _*_ __ __ __
    // ->
    // __ __ _*_ __ __
    // ->
    //...
    // __ __ __ __ _*_
    // Move the _*_ to the end and then remove that
    // element
    for (int i = index; i < ref->count - 1; i++) {
        array_swap(ref, index, index + 1);
    }

    // Removing simply means marking it as out-of-bounds
    // (possibly also shortening to save memory but that
    // is left for later)
    uint32_t last_index = ref->count - 1;
    void *removed = array_get(ref, last_index);
    array_set(ref, last_index, NULL);

    // Mark as invalid
    ref->count--;
    ref->len -= sizeof(void*);
    return removed;
}

void*
array_remove_first(array_ref ref) {
    return array_remove(ref, 0);
}

void*
array_remove_last(array_ref ref) {
    return array_remove(ref, (int)ref->count - 1);
}

array_ref
array_shallow_cpy(array_refc ref) {
    array_ref cpy = array_create();
    array_append_array(cpy, ref);
    return cpy;
}

array_ref
array_deep_cpy(array_refc ref, void*(*cpy)(void*)) {
    return array_map(ref, cpy);
}

array_ref
array_filter(array_refc ref, bool(*filter)(void*)) {
    if (!ref || !filter) {
        errno = EINVAL;
        return NULL;
    }
    array_ref cpy = array_create();
    for (int i = 0; i < array_count(ref); i++) {
        void *elem = array_get(ref, i);
        filter(elem) ? array_append(cpy, elem) : (void)0;
    }
    return cpy;
}

array_ref
array_map(array_refc ref, void*(*map)(void*)) {
    if (!ref || !map) {
        errno = EINVAL;
        return NULL;
    }
    array_ref cpy = array_create();
    for (int i = 0; i < array_count(ref); i++)
        array_append(cpy, map(array_get(ref, i)));
    return cpy;
}

void*
array_reduce(array_refc ref, void*(*reduce)(void*, void*)) {
    void *result = NULL;
    for (int i = 0; i < array_count(ref); i++)
        result = reduce(result, array_get(ref, i));
    return result;
}

void
array_for_each(array_refc ref, void(*each)(void*)) {
    if (!ref || !each) {
        errno = EINVAL;
        return;
    }
    uint32_t count = array_count(ref);
    for (int i = 0; i < count; i++)
        each(array_get(ref, i));
}