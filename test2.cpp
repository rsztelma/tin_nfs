#include <iostream>
#include "lib_nfs.h"


int main()
{
	int fds;
	struct stat dane;
	fds = mynfs_open((char *)"127.0.0.1", (char *)"mynfs_testing_plik.txt", O_RDONLY, 00666);
	mynfs_fstat((char *)"127.0.0.1", fds, &dane);
	mynfs_close((char *)"127.0.0.1", fds);
	std::cout << dane.st_uid << std::endl << dane.st_size;

	return 0;
}