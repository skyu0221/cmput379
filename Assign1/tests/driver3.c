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

	int            fd;
	unsigned char *instances;
	unsigned int   return_value, patlength,
	               loclength = 10;

	struct patmatch *locations = malloc( sizeof( struct patmatch )
	                                     * loclength * 3 );

	fd = open( "driver3file", O_CREAT | O_RDWR | O_TRUNC, 00777 );

	write( fd, "Try to test follow: alabula alabull alabulaalabula", 50 );

	patlength = strlen( argv[1] );
	memset( locations, 0, sizeof( locations ) );
	return_value = findpattern( argv[1], patlength, locations, loclength );

	printf( "test3\n" );
	printf( "This is for instances by using MMAP.\n" );
	printf( "\nPass 1\n" );
	printf( "Total Matches: %u\n", return_value );

	for ( unsigned int i = 0; i < MIN( loclength, return_value ); i++ ) {

		printf( "0x%.8x\t", locations[i].location );

		if ( locations[i].mode ) printf( "MEM_RO\n" );
		else printf( "MEM_RW\n" );
	}

	instances = (char *)mmap( NULL, sizeof( char ) * 50,
	                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );

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

	munmap( instances, sizeof( char ) * 50 );

	instances = (char *)mmap( NULL, sizeof( char ) * 50,
	                          PROT_READ, MAP_SHARED, fd, 0 );


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

	close( fd );
	munmap( instances, sizeof( char ) * 50);

	return 0;
}
