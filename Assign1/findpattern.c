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

			if ( index )
				current--;

			if ( index-- == patlength ) {
				counter++;

				//for ( int i = index; i >= 0; i-- )
					//printf( "%x | %c | %c\n", current - i, *(char * )(current - i), pattern[index - i] );
				printf( "%x\n+++\n", current - index );

				if ( counter <= loclength ) {

					locations[counter - 1].location = current - index;
					locations[counter - 1].mode     = mode;
				}
			}
		}
	}

	signal( SIGSEGV, SIG_DFL );
	signal( SIGBUS,  SIG_DFL );
	return counter;
}

void do_not_care () {

	unsigned int  page    = START_ADDR;
	unsigned char checker;

	struct sigaction act;

	act.sa_handler = handler;
	act.sa_flags   = 0;
	sigemptyset( &act.sa_mask );
	sigaction( SIGSEGV, &act, 0 );
	sigaction( SIGBUS,  &act, 0 );

	for ( ; page <= FINAL_ADDR - getpagesize(); page += getpagesize() ) {

		if ( sigsetjmp( env, 1 ) )
			continue;

		if ( !sigsetjmp( env, 1 ) )
			*(char *)page = checker;
	}

	signal( SIGSEGV, SIG_DFL );
	signal( SIGBUS,  SIG_DFL );
}
