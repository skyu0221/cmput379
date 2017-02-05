#include <stdio.h>
#include <stdlib.h>
#include "findpattern.h"
#include <string.h>


#define MIN(x, y) (((x) < (y)) ? (x) : (y))

// https://gcc.gnu.org/onlinedocs/cpp/Stringification.html
#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(s) #s
#ifndef P
#define P ""
#endif

int main(int argc, char *argv[]) {

    char *pattern = STRINGIFY(P);
    int loclength = 16, patlength, patterns_found, i;
    struct patmatch *locations = malloc(sizeof(struct patmatch) * loclength);
    struct patmatch *loc;
    patlength = strlen(pattern);

    printf("Test1\n");
    printf("Memory modified on the heap using malloc()\n");

    printf("Pass 1\n");

    patterns_found = findpattern(pattern, patlength, locations, loclength);
    printf("Found %d instances of the pattern.\n", patterns_found);
   
    for (loc = locations, i = 0; i < MIN(loclength, patterns_found); i++, loc++)
    {
	printf("Pattern found at %.8x, ", loc->location);
	if (loc->mode == 0)
		printf("Mode: MEM_RW\n");
	else
		printf("Mode: MEM_RO\n");
    }


    // make changes
    char *new_instance = malloc(patlength + 1);
    strcpy(new_instance, pattern);
    printf("New instance of pattern at %.x\n", (unsigned int) new_instance);
    
    printf("Pass 2\n");
    patterns_found = findpattern(pattern, patlength, locations, loclength);
    printf("Found %d instances of the pattern.\n", patterns_found);

    // print out where we found the pattern.
    for (loc = locations, i = 0; i < MIN(loclength, patterns_found); i++, loc++)
    {
	printf("Pattern found at %.8x, ", loc->location);
	if (loc->mode == 0)
		printf("Mode: MEM_RW\n");
	else
		printf("Mode: MEM_RO\n");
    }
    free(new_instance);
    printf("pattern length: %d\n", patlength);
    
    exit(0);	
}
