#pragma once

/**
 * @brief The entry point for plugins loaded into McMini
 * TODO: Supporting plugins in the future means handling dlopen() of C++
 * binaries. See the ld(2) man page for how McMini should be compiled (`-E`
 * and/or `--dynamic-list-cpp-typeinfo` and`--dynamic-list-cpp-new`)
 */
void mcmini_plugin_launch();
