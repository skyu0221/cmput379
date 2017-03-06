CMPUT 379 Winter 2017 - Assignment 1

Tianyu Zhang & Di Zhang

The findpattern() will try to search the whole memory (from 0x00000000 to
0xFFFFFFFF) and find all the match pattern with given pattern.

In our driver program, we suggest to use pattern "alabula" because all the
driver program will use this pattern multiple times. For specific, the value we
stored in memory is "Try to test follow: alabula alabull alabulaalabula".
For driver1 and driver2, additional value is "alaalabulalbulalabula".

Notice the expected result is based on only one pattern in the terminal. I.e.
run the driver by hand "./driver[1-3] alabula" or "make tests". If you are using
makefile "make test[1-3] p=alabula", actually "alabula" will be called three
times in the terminal. Check next section for more about makefile.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
makefile:

make tests p=<pattern> (default p=alabula if not reassign by user)
    This will clean the directory and compile all the files and run all drivers.
    The result will be saved in ./tests/test_results.txt. No result on stdout.

make test[1-3] p=<pattern> (default p=alabula if not reassign by user)
    This will just compile and run specific driver and print the result to
    stdout (screen).

make clean
    This will remove all compiled file and *.o file, test_results.txt and the
    file made by driver3.
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
driver1:
Place instances of the pattern on the heap to see if findPattern() finds them.

Currently, a good idea for our driver is using pattern "alabula". The reason of
choosing this as pattern is we want to avoid protential match in any other
memory space. We use MALLOC create a CHAR* with length of 51 (1 for '\0') and
we put texts in it makes it contains ONE pattern. Then we change the texts to
make it contains THREE pattern. At the end we use REALLOC make a copy of this
CHAR* and increase the size to 72 with a text. In this CHAR* it includes TWO 
patterns.

Start
-> MALLOC "Try to test follow: alabula alabull" (1 pattern total)
-> Append " alabulaalabula" (3 pattern total), now MALLOCed memory is full.
-> REALLOC the MALLOCed value with larger memory space and append
   "alaalabulalbulalabula" to the end (5 pattern total).

In general
First call findpattern() should return 2 (1 in text and 1 for pattern itself).
Second call findpattern() should return 4 (3 in text and 1 for pattern itself).
Third call findpattern() should return 9 (3 in first text and 
                                          5 in second text and
                                          1 for pattern itself).
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
driver2:
Create local instances of the pattern on the stack.

This setup is pretty similar to the driver1 but we put values in stack. We use
memset to initialize two texts to zeros. First put ONE pattern into first text,
then add TWO more pattern into first text, then put TWO pattern into second
text.
Notice for this driver the loclength is 5.

Start
-> *value1 = "Try to test follow: alabula alabull" (1 pattern total)
-> Append " alabulaalabula" to value1 (3 pattern total in value1).
-> *value2 = "alaalabulalbulalabula"(2 pattern in value2).

In general
First call findpattern() should return 2 (1 in text and 1 for pattern itself).
Second call findpattern() should return 4 (3 in text and 1 for pattern itself).
Third call findpattern() should return 6 (3 in first 7text and 
                                          2 in second text and
                                          1 for pattern itself).
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
driver3:
Map a file into memory using mmap(2). Then use mmap(2) to change the mode of the
memory you are mapping to.

First we write a text to a new file called "driver3file". If this file is
existed, will replace the file. The text contains THREE patterns. Here we will
do the first findpattern(). Then we will use mmap to map this file into RW, and
call findpattern() again. Then we will use mmap to change this file into RO, and
do the third findpattern().

Start
-> Write to file "Try to test follow: alabula alabull alabulaalabula"
   (3 pattern total. Appear in both buffer and file.)
-> Use mmap map the file into RW zone. (3 more pattern in RW).
-> Use mmap map the file into RO zone. (Same 3 pattern address with RW to RO).

In general
First call findpattern() should return 7 (3 in buffer 3 in file
                                          and 1 for pattern itself).
Second call findpattern() should return 10 (7 before and 3 RW).
Third call findpattern() should return 10 (7 before and 3 RO).
