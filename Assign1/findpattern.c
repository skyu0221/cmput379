#include "findpattern.h"

jmp_buf env;

unsigned int findpattern ( unsigned char   *pattern,
                           unsigned int     patlength,
                           struct patmatch *locations,
                           unsigned int     loclength ) {
    

    unsigned int page;
    //loop through pages and find pattern match (2^32-1)
    //segmentation fault handling?
}

int main(){
    
    findpattern(pattern, patlength, locations, loclength);
    return 0;
}
