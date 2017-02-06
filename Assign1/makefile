CC = gcc
CFLAGS = -lm -m32 -std=gnu99
p = alabula

tests: clean findpattern.o
	@cd tests/ && make compile
	@echo "\nRunning driver1."
	@cd tests/ && ( ./driver1 $(p) > test_results.txt )
	@echo "" >> ./tests/test_results.txt
	@echo "Running driver2.."
	@cd tests/ && ( ./driver2 $(p) >> test_results.txt )
	@echo "" >> ./tests/test_results.txt
	@echo "Running driver3..."
	@cd tests/ && ( ./driver3 $(p) >> test_results.txt )
	@echo "Done! Please check ./tests/test_results.txt"

test1:
	cd tests/ && make test1 p=$(p)

test2:
	cd tests/ && make test2 p=$(p)

test3:
	cd tests/ && make test3 p=$(p)

findpattern.o: findpattern.c findpattern.h
	@$(CC) $(CFLAGS) -c findpattern.c

clean:
	@rm -rf *.o && cd tests/ && make clean
