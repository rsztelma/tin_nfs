#include <iostream>
#include "lib_nfs.h"


int main()
{
    char *ip_addr = (char *)"172.18.77.72";
    std::cout << "test 5 usuniecie pliku" << '\n';
    mynfs_unlink(ip_addr, (char *)"mynfs_testing_plik.txt");
    std::cout << "test 5 zakonczony" << '\n';
    return 0;
}
