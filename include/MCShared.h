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
#define MC_EXTERN             extern "C"
#define MC_EXTERN_DECLS_BEGIN extern "C" {
#define MC_EXTERN_DECLS_END   }
#else
#define MC_EXTERN
#define MC_EXTERN_DECLS_BEGIN
#define MC_EXTERN_DECLS_END
#endif

#ifdef __cplusplus
#define MC_THREAD_LOCAL thread_local
#else
#define MC_THREAD_LOCAL __thread
#endif

#ifdef __cplusplus
#define MC_NO_RETURN [[noreturn]]
#else
#define MC_NO_RETURN __attribute__((__noreturn__))
#endif

#define MC_ASSERT(__X) assert(__X)

#ifdef __cplusplus
#define MC_FATAL_ON_FAIL(expr)                     \
  do {                                             \
    (static_cast<bool>(expr) ? void(0) : abort()); \
  } while (0)
#else
#define MC_FATAL_ON_FAIL(expr)    \
  do {                            \
    ((expr) ? void(0) : abort()); \
  } while (0)
#endif

#define MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(x, str)                \
  do {                                                              \
    if (!static_cast<bool>(x)) {                                    \
      mc_report_undefined_behavior(static_cast<const char *>(str)); \
    }                                                               \
  } while (0)

#define MC_REPORT_UNDEFINED_BEHAVIOR(str) \
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(false, str)

#if __GNUG__ && __GNUC__ < 5
#define MC_IS_TRIVIALLY_COPYABLE(T) __has_trivial_copy(T)
#else
#define MC_IS_TRIVIALLY_COPYABLE(T) \
  std::is_trivially_copyable<T>::value
#endif

#define MC_FATAL() abort()

#endif // DPOR_MCSHARED_H
