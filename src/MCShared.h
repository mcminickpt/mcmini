#ifndef DPOR_MCSHARED_H
#define DPOR_MCSHARED_H

#include <assert.h>
#include "MCConstants.h"

typedef uint64_t objid_t;
typedef objid_t MCObjectID;
typedef uint64_t MCTypeID;
typedef void *MCSystemID;

#ifndef __cplusplus
#   define MC_EXTERN
#   define MC_EXTERN_DECLS_BEGIN
#   define MC_EXTERN_DECLS_END
#else
#   define MC_EXTERN extern "C"
#   define MC_EXTERN_DECLS_BEGIN extern "C" {
#   define MC_EXTERN_DECLS_END }
#endif

#ifndef __cplusplus
#   define MC_THREAD_LOCAL thread_local
#else
#   define MC_THREAD_LOCAL __thread
#endif

#ifndef __cplusplus
#   define MC_NO_RETURN
#else
#   define MC_NO_RETURN [[noreturn]]
#endif

#define MC_ASSERT(__X) assert(__X)
#define MC_FAIL() MC_ASSERT(0)
#define MC_FATAL_ON_FAIL(expr) \
do {                            \
    (static_cast <bool> (expr) ? void (0) : abort()); \
} while(0)

#define MC_FATAL() abort()
#define MC_UNIMPLEMENTED() MC_ASSERT(false)

#define MC_STRUCT_DECL(type)             \
typedef struct type type;           \
typedef struct type *type##_t;      \
typedef struct type *type##_ref;    \
typedef const struct type *type##_refc;


#endif //DPOR_MCSHARED_H
