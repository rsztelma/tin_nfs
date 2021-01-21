#include "lib_nfs.h"
#include <iostream>

int main()
{
    int fds;
    char *tekst = (char *)malloc(sizeof(char)*50);
    char *ip_addr = (char *)"172.18.77.72";
std::cout << "test 3 otworzenie pliku i odczytanie z pliku na konsole" << '\n';
    fds = mynfs_open(ip_addr, (char *)"mynfs_testing_plik.txt", O_RDONLY, 00666);
    mynfs_read(ip_addr, fds, (void *)tekst, 45*sizeof(char));
    mynfs_close(ip_addr, fds);

	std::cout << tekst << std::endl;
    std::cout << "test 3 zakonczony" << '\n';

	return 0;
}
