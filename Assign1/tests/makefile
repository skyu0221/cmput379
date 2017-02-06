CC = gcc
CFLAGS = -lm -m32 -std=gnu99
FILES = ../findpattern.o
p = alabula

compile: compile1 compile2 compile3

compile1: driver1.c $(FILES)
	@$(CC) $(CFLAGS) driver1.c $(FILES) -o driver1

compile2: driver2.c $(FILES)
	@$(CC) $(CFLAGS) driver2.c $(FILES) -o driver2

compile3: driver3.c $(FILES)
	@$(CC) $(CFLAGS) driver3.c $(FILES) -o driver3

test1: compile1
	./driver1 $(p)

test2: compile2
	./driver2 $(p)

test3: compile3
	./driver3 $(p)

findpattern.o: ../findpattern.c ../findpattern.h
	@cd ../ && $(CC) $(CFLAGS) -c findpattern.c

clean:
	@rm -rf driver1 driver2 driver3 driver3file test_results.txt *.o
