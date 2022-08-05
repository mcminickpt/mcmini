# McMini
A bite-size C/C++ model checker using Dynamic Partial Order Reduction

## High-Level Overview of Functionality
McMini is an _extensible_ model checker which inspects
programs using _Dynamic Partial Order Reduction_ (or DPOR for
short), a technique for reducing the number of thread orderings 
that need to be searched. The algorithm is described in the seminal
2005 paper by Flannagan and Godefroid bearing the algorithm's title.

## Building McMini
McMini uses [CMake](https://cmake.org) to generate its targets. You can 
build McMini by running the following commands

```bash
git clone THIS_REPO

# Build with debug symbols. Use -DCMAKE_BUILD_TYPE=Release
# for speed optimizations and for benchmarks 
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug

# Build the targets
cmake --build build -j 10
```
CMake will place the generated binaries/libraries into directories
in the build tree corresponding to the directories in the source tree
