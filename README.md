# McMini
A bite-size C/C++ model checker using Dynamic Partial Order Reduction

## High-Level Overview of Functionality
McMini is an _extensible_ model checker which inspects
programs using _Dynamic Partial Order Reduction_ (or DPOR for
short), a technique for reducing the number of thread orderings
that need to be searched. The algorithm is described in the seminal
2005 paper by Flannagan and Godefroid bearing the algorithm's title.

## Building McMini
McMini can be built in either of two ways:
1. McMini can use traditional:  ./configure && make
2. If CMake is available, McMini can use [CMake](https://cmake.org)
to generate its targets.

# Build with configure/make

```bash
git clone THIS_REPO
cd mcmini
./configure && make -j10 && make check

# To then run under GDB:
make clean && make -j10 check-gdb
# For syntax with GDB and other targets, do:  make -n check-gdb

## Other options:
```bash
make clean
make dist
make debug  # Build with '-g3 -O0'
```

# Build with Cmake

```bash
git clone THIS_REPO
cd mcmini

# Build with debug symbols. Use -DCMAKE_BUILD_TYPE=Release
# for speed optimizations and for benchmarks
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug

# Build the targets
cmake --build build -j 10
```
CMake will place the generated binaries/libraries into directories
in the build tree corresponding to the directories in the source tree
