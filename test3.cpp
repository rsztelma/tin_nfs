#include "lib_nfs.h"
#include <iostream>

int main()
{
    int fds;
    char *tekst = (char *)malloc(sizeof(char)*50);
    fds = mynfs_open((char *)"127.0.0.1", (char *)"mynfs_testing_plik.txt", O_RDONLY, 00666);
    mynfs_read((char *)"127.0.0.1", fds, (void *)tekst, (strlen("Ala ma kota")+1)*sizeof(char));
    mynfs_close((char *)"127.0.0.1", fds);

	std::cout << tekst << std::endl;

	return 0;
}