CFLAGS=-g3 -O0 -pthread

%: %.cpp
	g++ $(CFLAGS) $< -o $@

%: %.c
	gcc $(CFLAGS) $< -o $@

rwwlock.o: rwwlock.c rwwlock.h
	gcc -c -fPIC $(CFLAGS) rwwlock.c -o rwwlock.o

librwwlock.so: rwwlock.o
	gcc -shared $(CFLAGS) -o librwwlock.so rwwlock.o

reader_writer_rwwlock: reader_writer_rwwlock.c librwwlock.so
	gcc $(CFLAGS) -Wl,-rpath=$(PWD) reader_writer_rwwlock.c -L$(PWD) -lrwwlock -o reader_writer_rwwlock

.PHONY: clean

clean:
	rm -f *.o *.so reader_writer_rwwlock
