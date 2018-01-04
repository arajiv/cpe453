CC=gcc
CFLAGS=-Wall -Werror -g -lcrypt

mksuid: mksuid.o
	$(CC) $(CFLAGS) mksuid.o -o $@ 

mksuid.o: mksuid.c
	$(CC) $(CFLAGS) -c mksuid.c 

clean:
	rm *.o mksuid
