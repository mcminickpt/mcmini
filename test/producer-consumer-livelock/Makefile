FILE = producer-consumer

CC = gcc

CPPFLAGS += -I../..
CFLAGS += -g3 -O0
LDLIBS += -lpthread

run: $(FILE)

# Create executable from producer-consumer.c and partial-solution.c
$(FILE): $(FILE).c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ \
		$(FILE).c partial-solution.c $(LDLIBS)

gdb: $(FILE)
	gdb $(FILE)

vi:
	vi $(FILE).c

emacs:
	emacs $(FILE).c

clean:
	rm -f $(FILE) a.out *~

# 'make' views $v as a make variable and expands $v into the value of v.
# By typing $$, make will reduce it to a single '$' and pass it to the shell.
# The shell will view $dir as a shell variable and expand it.
dist:
	dir=`basename $$PWD`; cd ..; tar cvf $$dir.tar ./$$dir; gzip $$dir.tar
	dir=`basename $$PWD`; ls -l ../$$dir.tar.gz
