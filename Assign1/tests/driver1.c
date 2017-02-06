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

	unsigned char *instances = malloc( sizeof( char ) * 50 );

	struct patmatch *locations = malloc( sizeof( struct patmatch ) * 5 );

	instances = "Try to test follow: alabula alabule alabulaalabula";

	patlength = strlen( argv[1] );

	return_value = findpattern( argv[1], patlength, locations, 5 );

	printf( "%d\n", return_value );

	return 0;
}
