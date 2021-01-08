#include "lib_nfs.h"

int main()
{
    int fds;
    char *tekst = (char *)"Ala ma kota";
    fds = mynfs_open((char *)"127.0.0.1", (char *)"mynfs_testing_plik.txt", O_CREAT, 00666);
    mynfs_write((char *)"127.0.0.1", fds, (void *)tekst, strlen(tekst) + 1);
    mynfs_close((char *)"127.0.0.1", fds);

    return 0;
}