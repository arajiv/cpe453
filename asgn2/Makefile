CC = gcc
CFLAGS = -Wall -g -fpic

num: numbersmain.o magic64.o  lwp.o
	$(CC) $(CFLAGS) numbersmain.o magic64.o lwp.o AlwaysZero.o -o $@

numbersmain.o: numbersmain.c magic64.S lwp.c AlwaysZero.c
	$(CC) $(CFLAGS) -c numbersmain.c lwp.c magic64.S AlwaysZero.c

prog: prog.o magic64.o lwp.o handsoff.o
	$(CC) $(CFLAGS) prog.o magic64.o lwp.o handsoff.o -o $@

prog.o: prog.c magic64.S lwp.c handsoff.c
	$(CC) $(CFLAGS) -c prog.c lwp.c magic64.S handsoff.c

main: main.o magic64.o lwp.o rr.o 
	$(CC) $(CFLAGS) main.o magic64.o lwp.o rr.o -o $@

main.o: main.c rr.c
	$(CC) $(CFLAGS) -c main.c rr.c 

test7: lwp.o magic64.o fiveprocsmain.o liblwp.so 
	$(CC) $(CFLAGS) -o $@ lwp.o magic64.o fiveprocsmain.o -L. -llwp

test5: lwp.o magic64.o oneprocmain.o liblwp.so 
	$(CC) $(CFLAGS) -o $@ lwp.o magic64.o oneprocmain.o -L. -llwp

liblwp.so: lwp.o magic64.o
	$(CC) $(CFLAGS) -shared -o $@ lwp.o magic64.o

lwp.o: lwp.c magic64.S
	$(CC) $(CFLAGS) -c lwp.c magic64.S

fiveprocsmain.o: fiveprocsmain.c
	$(CC) $(CFLAGS) -c fiveprocsmain.c -o fiveprocsmain.o

oneprocmain.o: oneprocmain.c
	$(CC) $(CFLAGS) -c oneprocmain.c -o oneprocmain.o
clean:
	rm -f *.o lwp
