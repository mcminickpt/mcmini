#ifndef DPOR_MCSHARED_H
#define DPOR_MCSHARED_H

#ifdef __cplusplus
#include <cassert>
#else
#include <assert.h>
#endif

#include "MCConstants.h"

typedef uint64_t objid_t;
typedef void *MCSystemID;

#ifdef __cplusplus
#   define MC_EXTERN extern "C"
#   define MC_EXTERN_DECLS_BEGIN extern "C" {
#   define MC_EXTERN_DECLS_END }
#else
#   define MC_EXTERN
#   define MC_EXTERN_DECLS_BEGIN
#   define MC_EXTERN_DECLS_END
#endif

#ifdef __cplusplus
#   define MC_THREAD_LOCAL thread_local
#else
#   define MC_THREAD_LOCAL __thread
#endif

#ifdef __cplusplus
#   define MC_NO_RETURN [[noreturn]]
#else
#   define MC_NO_RETURN
#endif

#define MC_ASSERT(__X) assert(__X)

#ifdef __cplusplus
#define MC_FATAL_ON_FAIL(expr) \
do {                            \
    (static_cast <bool> (expr) ? void (0) : abort()); \
} while(0)
#else
#define MC_FATAL_ON_FAIL(expr) \
do {                            \
    ((expr) ? void (0) : abort()); \
} while(0)
#endif

#define MC_FATAL() abort()
#define MC_STRUCT_DECL(type)             \
typedef struct type type;           \
typedef struct type *type##_t;      \
typedef struct type *type##_ref;    \
typedef const struct type *type##_refc;


#endif //DPOR_MCSHARED_H
