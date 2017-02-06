CC = gcc
CFLAGS = -lm -m32
p = alabula

tests: clean findpattern.o
	cd tests/ && make compile
	cd tests/ && ( ./driver1 $(p) | tee test_results.txt )
	echo "" >> ./tests/test_results.txt
	cd tests/ && ( ./driver2 $(p) | tee -a test_results.txt )
	echo "" >> ./tests/test_results.txt
	cd tests/ && ( ./driver3 $(p) | tee -a test_results.txt )

test1:
	cd tests/ && make test1 p=$(p)

test2:
	cd tests/ && make test2 p=$(p)

test3:
	cd tests/ && make test3 p=$(p)

findpattern.o: findpattern.c findpattern.h
	$(CC) $(CFLAGS) -c findpattern.c

clean:
	rm -rf output *.o && cd tests/ && make clean
