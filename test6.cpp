#include "lib_nfs.h"

int main()
{
    int fds;
    char *ip_addr = (char *)"172.18.77.72";
    char *tekst = (char *)"Ala ma kota";
    std::cout << "Test 6 - warinat 12 - blokowanie dostepu\n";
    fds = mynfs_open(ip_addr, (char *)"mynfs_testing_plik.txt", O_RDWR | O_CREAT, 00666);
    mynfs_write(ip_addr, fds, (void *)tekst, strlen(tekst));
    std::cout << "Potwierdz zamkniecie pliku wciskajac przycisk enter\n";
    getchar();
    mynfs_close(ip_addr, fds);
	std::cout << "Test 6 zakonczony\n";

	return 0;
}
