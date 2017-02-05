#include "../findpattern.h"
#include <stdio.h>

int main() {

	char alist[10] = [ 'a','b','c','d','a','e','f','g','a','b' ];

	printf( "%d\n", findpattern( alist, 10,  ) );

	

	return 0;
}
