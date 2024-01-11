#pragma once

#define MCMINI_INLINE
#define MCMINI_LIBRARY_ENTRY_POINT
#define MCMINI_EXPORT __attribute__((visibility(default)))
#define MCMINI_PRIVATE __attribute__((visibility(hidden)))

// TODO: Other defines here based on system types etc.
// TODO: Add "extern "C" where appropriate