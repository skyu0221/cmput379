CC = gcc
CFLAGS = -lm -m32

test1: findpattern.o
	cd ./tests && make

findpattern.o: findpattern.c findpattern.h
	$(CC) $(CFLAGS) -c findpattern.c

clean:
	rm -rf output *.o
