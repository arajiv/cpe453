CC = gcc
CFLAGS = -Wall -g -Wpedantic

pipeit: pipeit.o
	$(CC) $(CFLAGS)  pipeit.o -o pipeit

pipeit.o: pipeit.c
	$(CC) $(CFLAGS) -c pipeit.c

clean:
	rm -f *.o pipeit outfile
