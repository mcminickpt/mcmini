#ifndef DPOR_GMALSHARED_H
#define DPOR_GMALSHARED_H

#include <assert.h>
#include "GMALConstants.h"

typedef uint64_t objid_t;
typedef objid_t GMALObjectID;
typedef uint64_t GMALTypeID;
typedef void *GMALSystemID;

#ifndef __cplusplus
#   define GMAL_EXTERN
#   define GMAL_EXTERN_DECLS_BEGIN
#   define GMAL_EXTERN_DECLS_END
#else
#   define GMAL_EXTERN extern "C"
#   define GMAL_EXTERN_DECLS_BEGIN extern "C" {
#   define GMAL_EXTERN_DECLS_END }
#endif

#ifndef __cplusplus
#   define GMAL_THREAD_LOCAL thread_local
#else
#   define GMAL_THREAD_LOCAL __thread
#endif

#ifndef __cplusplus
#   define GMAL_NO_RETURN
#else
#   define GMAL_NO_RETURN [[noreturn]]
#endif

#define GMAL_ASSERT(__X) assert(__X)
#define GMAL_FAIL() GMAL_ASSERT(0)
#define GMAL_FATAL_ON_FAIL(expr) \
do {                            \
    (static_cast <bool> (expr) ? void (0) : abort()); \
} while(0)

#define GMAL_FATAL() abort()
#define GMAL_UNIMPLEMENTED() GMAL_ASSERT(false)

#define GMAL_STRUCT_DECL(type)             \
typedef struct type type;           \
typedef struct type *type##_t;      \
typedef struct type *type##_ref;    \
typedef const struct type *type##_refc;


#endif //DPOR_GMALSHARED_H
