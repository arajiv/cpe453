CC = gcc
CFLAGS = -Wall -g -fpic

malloc: lib/libmalloc.so lib64/libmalloc.so

intel-all: lib/libmalloc.so lib64/libmalloc.so

lib/libmalloc.so: lib malloc32.o
	$(CC) $(CFLAGS) -m32 -shared -o $@ malloc32.o

lib64/libmalloc.so: lib64 malloc64.o
	$(CC) $(CFLAGS) -m64 -shared -o $@ malloc64.o

lib:
	mkdir lib

lib64:
	mkdir lib64

malloc32.o: malloc.c
	$(CC) $(CFLAGS) -m32 -c malloc.c -o $@

malloc64.o: malloc.c
	$(CC) $(CFLAGS) -m64 -c malloc.c -o $@

prog64: lib64/libmalloc.so malloc64.o prog64.o lib64.o
	$(CC) $(CFLAGS) prog.o malloc64.o lib.o -o prog64 -L./lib64 -lmalloc

prog32: lib/libmalloc.so malloc32.o prog32.o lib32.o
	$(CC) $(CFLAGS) prog.o malloc32.o lib.o -m32 -o prog32 -L./lib -lmalloc

prog32.o: prog.c
	$(CC) $(CFLAGS) -c -m32 prog.c

lib32.o: lib.c
	$(CC) $(CFLAGS) -c -m32 lib.c

prog64.o: prog.c
	$(CC) $(CFLAGS) -c prog.c

lib64.o: lib.c
	$(CC) $(CFLAGS) -c lib.c

clean:
	rm -f *.o libmalloc.a asgn1
