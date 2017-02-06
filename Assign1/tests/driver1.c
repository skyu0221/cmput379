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

	unsigned int   return_value, patlength,
	               loclength = 5;
	unsigned char *instances = malloc( sizeof( char ) * 51 );
	struct patmatch *locations = malloc( sizeof( struct patmatch )
	                                     * loclength );
	strcpy( instances, "Try to test follow: alabula alabull" );

	patlength = strlen( argv[1] );
	memset( locations, 0, sizeof( locations ) );
	return_value = findpattern( argv[1], patlength, locations, loclength );

	printf( "test1\n" );
	printf( "This is for instances in HEAP.\n" );
	printf( "Expected result is 2, 4, 9.\n" );
	printf( "\nPass 1\n" );
	printf( "Total Matches: %u\n", return_value );

	for ( unsigned int i = 0; i < MIN( loclength, return_value ); i++ ) {

		printf( "0x%.8x\t", locations[i].location );

		if ( locations[i].mode ) printf( "MEM_RO\n" );
		else printf( "MEM_RW\n" );
	}

	strcat( instances, " alabulaalabula" );
	memset( locations, 0, sizeof( locations ) );
	return_value = findpattern( argv[1], patlength, locations, loclength );

	printf( "\nPass 2\n" );
	printf( "Total Matches: %u\n", return_value );

	for ( unsigned int i = 0; i < MIN( loclength, return_value ); i++ ) {

		printf( "0x%.8x\t", locations[i].location );

		if ( locations[i].mode ) printf( "MEM_RO\n" );
		else printf( "MEM_RW\n" );
	}

	instances = realloc( instances, sizeof( char ) * 72 );
	strcat( instances, "alaalabulalbulalabula" );

	memset( locations, 0, sizeof( locations ) );
	return_value = findpattern( argv[1], patlength, locations, loclength );

	printf( "\nPass 3\n" );
	printf( "Total Matches: %u\n", return_value );

	for ( unsigned int i = 0; i < MIN( loclength, return_value ); i++ ) {

		printf( "0x%.8x\t", locations[i].location );

		if ( locations[i].mode ) printf( "MEM_RO\n" );
		else printf( "MEM_RW\n" );
	}

	free(instances);

	return 0;
}
