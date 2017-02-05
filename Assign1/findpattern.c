#include "findpattern.h"

jmp_buf env;

unsigned int findpattern ( unsigned char   *pattern,
                           unsigned int     patlength,
                           struct patmatch *locations,
                           unsigned int     loclength ) {
<<<<<<< HEAD

	unsigned int counter = 0;

	return counter;
=======
    

    unsigned int page;
    //loop through pages and find pattern match (2^32-1)
    //segmentation fault handling?
}

int main(){
    
    findpattern(pattern, patlength, locations, loclength);
    return 0;
>>>>>>> 2d5978df7e4bb6a1f766162fad06b29aa5001576
}
