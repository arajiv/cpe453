CC = gcc
CFLAGS = -Wall -g -fpic

asgn1: asgn1.o malloc.o libmalloc.a
	$(CC) $(CFLAGS) -o asgn1 asgn1.o malloc.o libmalloc.a

asgn1.o: asgn1.c
	$(CC) $(CFLAGS) -c asgn1.c

malloc.o: malloc.c
	$(CC) $(CFLAGS) -c malloc.c

libmalloc.a: malloc.o
	ar -r libmalloc.a malloc.o

clean:
	rm -f *.o libmalloc.a asgn1
