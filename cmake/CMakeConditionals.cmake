# Adds the given subdirectory if the specified
# condition is satisfied
#
#   COND: A variable which is evaluated
#   as a boolean to determine whether or
#   or not the given subdirectory should
#   be added from the calling location.
#   If evaluated to anything cmake-equivalent
#   to TRUE, a call to `add_subdirectory()` is made
#
#   REASON: A string describing why the
#   `add_subdirectory()` call is optional. For example,
#   the subdirectory may include code that only
#   compiles on a certain platform
#
#   DIRECTORY: The subdirectory relative to the
#   caller's location to be added if COND evaluates
#   to anything cmake-equivalent to TRUE
function(add_subdirectory_if COND REASON DIRECTORY)
  if (${COND})
    message(STATUS "Processing conditional subdirectory \"${DIRECTORY}\"...")
    add_subdirectory("${DIRECTORY}" "${ARGN}")
  else()
    message(STATUS "Ignoring conditional subdirectory " "\"${DIRECTORY}\": ${REASON}")
  endif()
endfunction()

# Includes the given CMake module if the specified
# condition is satisfied
#
#   COND: A variable which is evaluated
#   as a boolean to determine whether or
#   or not the given module should
#   be added from the calling location.
#   If evaluated to anything cmake-equivalent
#   to TRUE, a call to `include()` is made
#
#   REASON: A string describing why the
#   `include()` call is optional. For example,
#   the CMake module may handle configuration
#   for a specific platform
#
#   MODULE_NAME: The name of the CMake module
#   to include if the given condition is satisfied
function(include_if COND REASON MODULE_NAME)
  if (${COND})
    message(STATUS "Including conditional cmake module \"${MODULE_NAME}\"...")
    include("${MODULE_NAME}" "${ARGN}")
  else()
    message(STATUS "Ignoring conditional cmake module " "\"${MODULE_NAME}\": ${REASON}")
  endif()
endfunction()
