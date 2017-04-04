#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define MIN( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define MAX( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#define POWOF2( n ) ( ( n & n - 1 ) == 0 )

void exit_with_error( char* error_message ) {

	printf( "Error: %s\n", error_message );
	exit(1);
}

int main( int argc, char *argv[] ) {

	// check the initial arguments
	// check number of arguments
	if ( argc < 8 )
		exit_with_error( "Number of arguments are wrong" );

	//tvm379 pgsize tlbentries {g|p} quantum physpages {f|l} trace1 trace2 ...
	long pgsize     = atol( argv[1] ),
	     tlbentries = atol( argv[2] ),
	     gp         =      *argv[3],
	     quantum    = atol( argv[4] ),
	     physpages  = atol( argv[5] ),
	     fl         =      *argv[6];

	printf( "%ld\n", pgsize );

	// check if pgsize is a power of 2 or in range
	if ( pgsize < 16 || pgsize > 65536 || !POWOF2( pgsize ) )
		exit_with_error( "pagesize is not a power of 2 or not in 16~65536" );

    // check tlbentries is a power of 2 or in range
	if ( tlbentries < 8 || tlbentries > 256 || !POWOF2( tlbentries ) )
        exit_with_error( "tlbentries is not a power of 2 or not in 8~256" );

    // check gp is 'g' or 'p'
	if ( gp != 'g' && gp != 'p' )
        exit_with_error( "flag for TLB entries unknown" );

    // check quantum is positive
	if ( quantum <= 0 )
        exit_with_error( "quantum is not a positive number" );

    // check physpages is in range
	if ( physpages > 1000000 || physpages < 0 )
        exit_with_error( "physpages is not in 0~1000000" );

    // check fl is 'f' or 'l'
	if ( fl != 'f' && fl != 'l' )
        exit_with_error( "flag for page eviction policy unknown" );
}
