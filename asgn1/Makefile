CC = gcc
CFLAGS = -Wall -g -fpic

intel-all: lib/libmalloc.so lib64/libmalloc.so

malloc32.o: malloc.c
	$(CC) $(CFLAGS) -m32 -c malloc.c -o $@

lib/libmalloc.so: malloc32.o
	$(CC) $(CFLAGS) -m32 -shared -o $@ malloc32.o

lib64/libmalloc.so: lib64 malloc64.o
	$(CC) $(CFLAGS) -m64 -shared -o $@ malloc64.o

lib:
	mkdir lib

lib64:
	mkdir lib64

asgn1: asgn1.o malloc64.o lib64/libmalloc.so
	$(CC) $(CFLAGS) -o asgn1 asgn1.o malloc64.o -L./lib64 -lmalloc

#asgn1.o: asgn1.c
#	$(CC) $(CFLAGS) -c asgn1.c

malloc64.o: malloc.c
	$(CC) $(CFLAGS) -m64 -c malloc.c -o $@

#libmalloc.a: malloc.o
#	ar -r libmalloc.a malloc.o

clean:
	rm -f *.o libmalloc.a asgn1
