#include "lib_nfs.h"

int main()
{
    int fds;
    char *ip_addr = (char *)"172.18.77.72";
    char *tekst = (char *)"Ala ma kota";
    std::cout << "Test 1 - tworzenie pliku i zapisywanie do pliku\n";
    fds = mynfs_open(ip_addr, (char *)"mynfs_testing_plik.txt", O_RDWR | O_CREAT, 00666);
    mynfs_write(ip_addr, fds, (void *)tekst, strlen(tekst));
    mynfs_close(ip_addr, fds);
	std::cout << "Test 1 zakonczony\n";

	return 0;
}
