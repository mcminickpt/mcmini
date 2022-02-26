#ifndef DPOR_DECL_H
#define DPOR_DECL_H

#define thread_local __thread

#define STRUCT_DECL(type)             \
typedef struct type type;           \
typedef struct type *type##_t;      \
typedef struct type *type##_ref;    \
typedef const struct type *type##_refc;

#define TYPES_DECL(state, ...) \
typedef enum state##_type { __VA_ARGS__ } state##_type; \

#define STATES_DECL(type, ...) \
typedef enum type##_state { __VA_ARGS__ } type##_state; \

#define MEMORY_API_DECL(type) \
type##_ref type##_alloc(void); \
type##_ref type##_create(void); \
type##_ref type##_copy(type##_refc); \
void type##_destroy(type##_ref);

#define MEMORY_ALLOC_DEF_DECL(type) \
inline type##_ref type##_alloc(void) {\
    return malloc(sizeof(type));    \
}

#define PRETTY_PRINT_DECL(type) \
void type##_pretty(type##_refc); \
void type##_pretty_off(type##_refc, unsigned int off);

#define PRETTY_PRINT_DEF_DECL(type) \
inline void type##_pretty(type##_refc ref) {   \
    type##_pretty_off(ref, 0u);          \
}


#endif //DPOR_DECL_H
