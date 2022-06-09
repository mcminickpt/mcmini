MCMINI_ROOT=../..

PYTHON=$(shell ls /usr/bin/python3 /usr/bin/python3 2>/dev/null |head -1)
EXECUTABLES=$(patsubst %.c,%,$(wildcard *.c)) \
            $(patsubst %.cpp,%,$(wildcard *.cpp))

# '-I${MCMINI_ROOT}/src' is for MCMINI.h, etc.
CFLAGS=-g3 -O0 -I${MCMINI_ROOT}/src -pthread
CXXFLAGS=-g3 -O0 -I${MCMINI_ROOT}/src -pthread

default: ${EXECUTABLES}

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
