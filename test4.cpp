#include <iostream>
#include "lib_nfs.h"


int main()
{
    
    mynfs_unlink((char *)"127.0.0.1", (char *)"plik1.txt");

    return 0;
}
