#include "lib_nfs.h"

int main()
{
    int fds;
    char *ip_addr = (char *)"127.0.0.1";
    char *tekst = (char *)"Ala ma kota";
    fds = mynfs_open(ip_addr, (char *)"mynfs_testing_plik.txt", O_RDWR | O_CREAT, 00666);
    mynfs_write(ip_addr, fds, (void *)tekst, strlen(tekst));
    mynfs_close(ip_addr, fds);
	std::cout << "test 1 finished" << '\n';

	return 0;
}