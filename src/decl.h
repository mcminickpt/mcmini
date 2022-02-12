#ifndef DPOR_DECL_H
#define DPOR_DECL_H

#define TYPE_DECL(type)             \
typedef struct type type;           \
typedef struct type *type##_t;      \
typedef struct type *type##_ref;    \
typedef const struct type *type##_refc;

#endif //DPOR_DECL_H
