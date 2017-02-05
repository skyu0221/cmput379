#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

#define MEM_RW 0
#define MEM_RO 1
#define START_ADDR 0x00000000
#define FINAL_ADDR 0xFFFFFFFF

struct patmatch {
	unsigned int  location;
	unsigned char mode; /* MEM_RW, or MEM_RO */
};

void handler ( int sig );

unsigned int findpattern ( unsigned char   *pattern,
                           unsigned int     patlength,
                           struct patmatch *locations,
                           unsigned int     loclength );
