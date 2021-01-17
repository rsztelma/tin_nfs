#include "lib_nfs.h"

int main()
{
    int fds;
    char *tekst = (char *)" wielkiego psa";
    char *ip_addr = (char *)"127.0.0.1";
    fds = mynfs_open(ip_addr, (char *)"mynfs_testing_plik.txt", O_RDWR | O_CREAT, 00666);
    mynfs_lseek(ip_addr, fds, 6, SEEK_SET);
    mynfs_write(ip_addr, fds, (void *)tekst, strlen(tekst));
    mynfs_close(ip_addr, fds);
	std::cout << "test 2 finished" << '\n';

	return 0;
}