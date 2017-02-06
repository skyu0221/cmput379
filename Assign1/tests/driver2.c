#include "../findpattern.c"
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MIN(a,b) (((a)<(b))?(a):(b))

int main ( int argc, char *argv[] ) {

	unsigned int  return_value, patlength,
	              loclength = 5;
	unsigned char instances[51], even_more[22];

	memset( instances, 0, sizeof( instances ) );
	memset( even_more, 0, sizeof( even_more ) );

	struct patmatch *locations = malloc( sizeof( struct patmatch )
	                                     * loclength * 3 );

	strcpy( instances, "Try to test follow: alabula alabull" );

	patlength = strlen( argv[1] );
	memset( locations, 0, sizeof( locations ) );
	return_value = findpattern( argv[1], patlength, locations, loclength );

	printf( "test2\n" );
	printf( "This is for instances in STACK. This driver loclength is 5.\n" );
	printf( "\nPass 1\n" );
	printf( "Total Matches: %u\n", return_value );

	for ( unsigned int i = 0; i < MIN( loclength, return_value ); i++ ) {

		printf( "0x%.8x\t", locations[i].location );

		if ( locations[i].mode ) printf( "MEM_RO\n" );
		else printf( "MEM_RW\n" );
	}

	strcat( instances, " alabulaalabula" );
	return_value = findpattern(  argv[1], patlength,
	                            &locations[loclength], loclength );

	printf( "\nPass 2\n" );
	printf( "Total Matches: %u\n", return_value );

	for ( unsigned int i = 0; i < MIN( loclength, return_value ); i++ ) {

		printf( "0x%.8x\t", locations[i + loclength].location );

		if ( locations[i + loclength].mode )
			printf( "MEM_RO\t" );
		else
			printf( "MEM_RW\t" );

		unsigned int j = 0, end = 0;

		while ( j < loclength && !end ) {

			if ( locations[i + loclength].location != locations[j].location )
				j++;
			else {

				if ( locations[i + loclength].mode == locations[j].mode ) {
					printf( "U\n" );
					end = 1;
				} else {
					printf( "C\n" );
					end = 1;
				}
			}
		}

		if ( !end )
			printf( "N\n" );
	}

	strcpy( even_more, "alaalabulalbulalabula" );

	return_value = findpattern(  argv[1], patlength,
	                            &locations[ loclength * 2 ], loclength );

	printf( "\nPass 3\n" );
	printf( "Total Matches: %u\n", return_value );

	for ( unsigned int i = 0; i < MIN( loclength, return_value ); i++ ) {

		printf( "0x%.8x\t", locations[i + loclength * 2].location );

		if ( locations[i + loclength * 2].mode )
			printf( "MEM_RO\t" );
		else
			printf( "MEM_RW\t" );

		unsigned int j = loclength, end = 0;

		while ( j < loclength * 2 && !end ) {

			if ( locations[i + loclength * 2].location !=
			                                             locations[j].location )
				j++;
			else {

				if ( locations[i + loclength * 2].mode == locations[j].mode ) {
					printf( "U\n" );
					end = 1;
				} else {
					printf( "C\n" );
					end = 1;
				}
			}
		}

		if ( !end )
			printf( "N\n" );
	}

	return 0;
}
