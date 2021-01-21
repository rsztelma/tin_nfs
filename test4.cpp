#include <iostream>
#include "lib_nfs.h"


int main()
{
	int fds;
	struct stat dane;
	char *ip_addr = (char *)"172.18.77.72";
    std::cout << "test 4 - fstat - wyswietlanie informacji o pliku" << '\n';
	fds = mynfs_open(ip_addr, (char *)"mynfs_testing_plik.txt", O_WRONLY, 00666);
	mynfs_fstat(ip_addr, fds, &dane);
	mynfs_close(ip_addr, fds);
	std::cout << "rozmiar: " << dane.st_size << " bajtow\n" << "ID wlasciciela:" << dane.st_uid << '\n';
	std::cout << "test 4 zakonczony" << '\n';

	return 0;
}
