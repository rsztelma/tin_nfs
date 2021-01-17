#include <iostream>
#include "lib_nfs.h"


int main()
{
    char *ip_addr = (char *)"127.0.0.1";
    mynfs_unlink(ip_addr, (char *)"mynfs_testing_plik.txt");
    std::cout << "test 5 finished" << '\n';
    return 0;
}
