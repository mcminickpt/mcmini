MCMINI_ROOT=../..

PYTHON=$(shell ls /usr/bin/python3 /usr/bin/python3 2>/dev/null |head -1)
EXECUTABLES=$(patsubst %.c,%,$(wildcard *.c)) \
            $(patsubst %.cpp,%,$(wildcard *.cpp))
MC_EXECUTABLES=$(patsubst %.c,mc_%,$(wildcard *.c)) \
               $(patsubst %.cpp,mc_%,$(wildcard *.cpp))

# '-I${MCMINI_ROOT}/src' is for mcmini.h, etc.
## On WSL-2 (Nov., 2022), -gdwarf-4 must be added to expand macro definitions.
CFLAGS=-g3 -gdwarf-4 -O0 -I${MCMINI_ROOT}/src -pthread --std=c99
CXXFLAGS=-g3 -gdwarf-4 -O0 -I${MCMINI_ROOT}/src -pthread

default: ${EXECUTABLES}

# This creates the mc_*.c version, but it fails to compile due to a bug
# because the .h files invoked use C++ instead of C.
# Do you want to use g++ instead of gcc here?
# If so, translate to .cpp file, and cast any malloc to <static_*?
#   inside mc_internalize.py
mc_executables: ${MC_EXECUTABLES}

%: %.cpp
	g++ ${CFLAGS} $< -o $@

%: %.c
	gcc ${CFLAGS} $< -o $@

mc_%.cpp: %.cpp
	${PYTHON} mc_internalize.py $<
mc_%.c: %.c
	${PYTHON} mc_internalize.py $<

# -----------------------------------
# You probably don't need these rules.  But here are more examples.
%.o: %.cpp
	g++ -o ${CXXFLAGS} $< -o $@

%.exe: %.cpp
	g++ ${CXXFLAGS} $< -o $@

clean:
	rm -f mc_*.c mc_*.cpp ${EXECUTABLES}
