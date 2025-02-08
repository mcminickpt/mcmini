# McMini: A small, extensible model checker

## Table of Contents

- [Usage](#usage)
- [High-Level Overview of Functionality](#high-level-overview-of-functionality)
- [Installation:  Building McMini](#installation--building-mcmini)
- [License](#license)

# Usage

* `./mcmini ./a.out <ARGS>`
* `./mcmini -m20 -f -q ./a.out <ARGS>`
* `./mcmini-gdb -m20 -f -q ./a.out <ARGS>`
* [ See: [Read the docs](https://mcmini-doc.readthedocs.io) ]

# High-Level Overview of Functionality

McMini is a small, extensible model checker which inspects multithreaded
programs using Dynamic Partial Order Reduction (or DPOR for short).
The DPOR algorithm was described in the seminal 2005 paper by Flannagan
and Godefroid bearing the algorithm's title.  McMini uses DPOR to search
the initial trace of all thread schedules for a multithreaded program.
Within this constraint, McMini  detects deadlock, segfaults, assertion
failures, and other errors.

Unlike some model checkers, it runs directly on a Linux binary executable.
It is as easy to use as:

>   `mcmini ./a.out` <ARGS>`

McMini can be used standalone, but also within GDB, so as, for example,
to examine a thread schedule leading to deadlock.

>   `mcmini-gdb ./a.out` <ARGS>`

When using GDB, be sure to compile your target (e.g., "a.out") with
debugging (`-g` or `-g3`).  McMini adds extra GDB commands, such as
`mcmini forward` (next multithreaded operation), `mcmini back` (last
multithreaded operation), `mcmini printTransitions` (list all previous
multithreaded operations), and more.

Below is a screenshot of a McMini session within GDB.

![Screenshot of a McMini session](https://github.com/mcminickpt/mcmini-doc/docs/source/usage/transitions-middle-subtle-small.png?raw=true)

If you'd like to try out McMini, then:
* [Read the docs](https://mcmini-doc.readthedocs.io); and
* Download and install McMini: `git clone https://github.com/mcminickpt/mcmini.git`

# Installation:  Building McMini
McMini can be built in either of two ways:

1. McMini can use traditional:  `./configure && make`
2. If CMake is available, McMini can use [CMake](https://cmake.org)
to generate its targets.

## Build with configure/make

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

## Build with Cmake

*(NOTE:  The cmake build is no longer actively maintained,
and is not guaranteed to work.)*

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

# License

McMini is free software. It is distributed under the GPLv3 license
(see [LICENSE.md](LICENSE.md)).
