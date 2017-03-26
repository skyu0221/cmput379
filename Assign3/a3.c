#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int powerOf2(unsigned int number){
    unsigned int power=1;
    while(power<number && power<65536){
        power *= 2;
    }
    return (number==power);
}

int main( int argc, char *argv[]) {
    //tvm379 pgsize tlbentries {g|p} quantum physpages {f|l} trace1 trace2 ...
    unsigned int pgsize=atoi(argv[1]);
    unsigned int tlbentries=atoi(argv[2]);
    unsigned int physpages=atoi(argv[5]);

    //check number of arguments
    if ( argc < 6 ) {
		printf( "Server: Number of arguments are wrong\n" );
		exit(1);
	}

    //check if pgsize is a power of 2 or in range
    if (pgsize<16 || pgsize>65536 || powerOf2(pgsize)!=1){
        printf( "pagesize is not a power of 2\n");
        exit(1);
    }

    //check tlbentries
    if (tlbentries<8 || tlbentries>256 || powerOf2(tlbentries)!=1){
        printf( "tlbentries is not a power of 2\n");
        exit(1);
    }

    //physpages
    if (physpages>1000000) {
        printf("physpages too large\n");
        exit(1);
    }
}
