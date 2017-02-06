#include "../findpattern.c"
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main ( int argc, char *argv[] ) {

	unsigned int   return_value, patlength;

	unsigned char instances[51], even_more[22];

	memset( instances, 0, sizeof( instances ) );
	memset( even_more, 0, sizeof( even_more ) );

	struct patmatch *locations = malloc( sizeof( struct patmatch ) * 5 );

	strcpy( instances, "Try to test follow: alabula alabull" );

	patlength = strlen( argv[1] );
	memset( locations, 0, sizeof( locations ) );
	return_value = findpattern( argv[1], patlength, locations, 5 );
	printf( "%d | %p\n", return_value, instances );

	strcat( instances, " alabulaalabula" );
	memset( locations, 0, sizeof( locations ) );
	return_value = findpattern( argv[1], patlength, locations, 5 );
	printf( "%d | %p\n", return_value, instances );

	strcpy( even_more, "alaalabulalbulalabula" );

	memset( locations, 0, sizeof( locations ) );
	return_value = findpattern( argv[1], patlength, locations, 5 );
	printf( "%d | %p\n", return_value, instances );

	return 0;
}
