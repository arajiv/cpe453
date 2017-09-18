CC=gcc

pipeit: pipeit.o
	$(CC) pipeit.o -o pipeit

pipeit.o: pipeit.c
	$(CC) -c pipeit.c

clean:
	rm -f *.o pipeit outfile
