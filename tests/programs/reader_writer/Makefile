CFLAGS=-g3 -O0 -pthread

%: %.cpp
	g++ ${CFLAGS} $< -o $@

%: %.c
	gcc ${CFLAGS} $< -o $@

# -----------------------------------
# You probably don't need these rules.  But here are more examples.
%.o: %.cpp
	g++ -o ${CFLAGS} $< -o $@

%.exe: %.cpp
	g++ ${CFLAGS} $< -o $@

rwwlock-lib:
	gcc -c -Wall -fpic rwwlock.c
	gcc -shared -Wall -o librwwlock.so rwwlock.o
	rm rwwlock.o

rwwlock: rwwlock-lib
	gcc -g3 -O0 -Wl,-rpath,'$$0RIGIN' reader_writer_rwwlock.c -L. -lrwwlock -pthread -o reader_writer_rwwlock
