#include "../findpattern.c"
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main ( int argc, char *argv[] ) {

	int            fd;
	unsigned char *instances;
	unsigned int   return_value, patlength;

	struct patmatch *locations = malloc( sizeof( struct patmatch ) * 5 );

	fd = open( "driver3file", O_CREAT | O_RDWR | O_TRUNC, 00777 );

	write( fd, "Try to test follow: alabula alabull alabulaalabula", 50 );

	patlength = strlen( argv[1] );
	memset( locations, 0, sizeof( locations ) );
	return_value = findpattern( argv[1], patlength, locations, 5 );
	printf( "%d | %p\n", return_value, instances );

	instances = (char *)mmap( NULL, sizeof( char ) * 50,
	                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );

	memset( locations, 0, sizeof( locations ) );
	return_value = findpattern( argv[1], patlength, locations, 5 );
	printf( "%d | %p\n", return_value, instances );

	munmap( instances, sizeof( char ) * 50 );

	instances = (char *)mmap( NULL, sizeof( char ) * 50,
	                          PROT_READ, MAP_SHARED, fd, 0 );


	memset( locations, 0, sizeof( locations ) );
	return_value = findpattern( argv[1], patlength, locations, 5 );
	printf( "%d | %p\n", return_value, instances );

	close( fd );
	munmap( instances, sizeof( char ) * 50);

	return 0;
}
