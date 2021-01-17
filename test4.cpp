#include <iostream>
#include "lib_nfs.h"


int main()
{
	int fds;
	struct stat dane;
	char *ip_addr = (char *)"127.0.0.1";
	fds = mynfs_open(ip_addr, (char *)"mynfs_testing_plik.txt", O_WRONLY, 00666);
	mynfs_fstat(ip_addr, fds, &dane);
	mynfs_close(ip_addr, fds);
	std::cout << "rozmiar: " << dane.st_size << std::endl << "ID wlasciciela:" << dane.st_uid << '\n';
	std::cout << "test 4 finished" << '\n';

	return 0;
}