CFLAGS=-O3 -pthread

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
