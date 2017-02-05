#include "findpattern.h"

sigjmp_buf env;

void handler ( int sig ) {

	siglongjmp( env, 1 );
}

unsigned int findpattern ( unsigned char   *pattern,
                           unsigned int     patlength,
                           struct patmatch *locations,
                           unsigned int     loclength ) {

	unsigned int  page    = START_ADDR,
	              counter = 0,
	              current, index;
	unsigned char checker, mode;

	struct sigaction act;

	act.sa_handler = handler;
	act.sa_flags   = 0;
	sigemptyset( &act.sa_mask );
	sigaction( SIGSEGV, &act, 0 );
	sigaction( SIGBUS,  &act, 0 );

	for ( ; page <= FINAL_ADDR - getpagesize(); page += getpagesize() ) {

		if ( !sigsetjmp( env, 1 ) )
			checker = *(char *)page;
		else
			continue;

		mode = MEM_RW;

		if ( !sigsetjmp( env, 1 ) )
			*(char *)page = checker;
		else
			mode = MEM_RO;

		for ( current = page; current < page + getpagesize(); current++  ) {

			index = 0;

			while ( *(char *)current == pattern[index] && index < patlength ) {

				index++;
				current++;
			}

			if ( index == patlength )
				counter++;

			if ( counter <= loclength ) {

				locations[counter].location = current;
				locations[counter].mode     = mode;
			}
		}
	}

	return counter;
    //loop through pages and find pattern match (2^32-1)
    //segmentation fault handling ?
}
