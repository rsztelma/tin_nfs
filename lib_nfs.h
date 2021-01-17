#ifndef NFSLIB_HPP
#define NFSLIB_HPP

#include <map>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdint>
#include <sys/socket.h>
#include <cstring>
#include <fcntl.h>
#include <sys/file.h>
#include <iostream>

#define MYNFS_PORT_NUMBER 9999

int mynfs_open(char *host, char *path, int oflag, int mode);  //1
int mynfs_read(char *host, int fds, void *buf, int count);    //2
int mynfs_write(char *host, int fds, void *buf, int count);   //3
int mynfs_lseek(char *host, int fds, int offset, int whence); //4
int mynfs_close(char *host, int fds);                         //5
int mynfs_unlink(char *host, char *path);                     //6
int mynfs_fstat(char *host, int fds, struct stat *buf);       //7

extern int mynfs_error;                               //Kod błędu funkcji. Wartości dodatnie mają takie same znaczenia, jak dla zmiennej errno.
                                                      //-1 --- błąd tworzenia gniazda (socket())
                                                      //-2 --- błąd połączenia (connect())
                                                      //-3 --- błąd wysyłania (write())
                                                      //-4 --- błąd odbioru (read())
                                                      //-5 --- błąd - serwer odzrucił połączenie
                                                      //-6 --- błąd - próba wykonania działania na serwerze, z którym nie zostało nawiązane połączenie
                                                      //-7 --- nieoczekiwane zakończenie połączenia z serwerem
                                                      //-8 --- przekroczenie limitu otwartych plików na danym serwerze
extern std::map<char *, int> mynfs_open_count;        //Tu przechowywana jest ilość otwartych plików na danym serwerze.
extern std::map<char *, int> mynfs_socket_descriptor; //Tu przechowywany jest deskryptor gniazda połączonego z danym serwerem.

struct mynfs_open_request
{
    uint16_t path_length; //długość ścieżki pliku
    int16_t oflag;        //parametr oflag funkcji
    int16_t mode;         //parametr mode funkcji
    char *path;           //ścieżka pliku
};

struct mynfs_open_response
{
    int32_t return_value; //deskryptor pliku lub kod błędu  (0 --- connection refused)
};

struct mynfs_read_request
{
    int32_t file_descriptor; //deskryptor pliku
    int16_t data_size;       //ilość danych do przeczytania (parametr count funkcji)
};

struct mynfs_read_response
{
    int16_t return_value; //liczba przeczytanych bajtów danych lub kod błędu
    int16_t data_size;    //rozmiar przesyłanych przez serwer danych
    void *data;           //przesłane przez serwer dane
};

struct mynfs_write_request
{
    int32_t file_descriptor; //deskryptor pliku
    int16_t data_size;       //ilość danych do zapisania (parametr count funkcji)
    void *data;              //dane do zapisania
};

struct mynfs_write_response
{
    int16_t return_value; //liczba zapisanych bajtów lub kod błędu
};

struct mynfs_lseek_request
{
    int32_t file_descriptor; //deskryptor pliku
    int16_t offset;          //parametr offset funkcji
    int16_t whence;          //parametr whence funkcji
};

struct mynfs_lseek_response
{
    int16_t return_value; //nowa pozycja w pliku lub kod błędu
};

struct mynfs_close_request
{
    int32_t file_descriptor; //deskryptor pliku
};

struct mynfs_close_response
{
    int16_t return_value; //0 w przypadku powodzenia lub kod błędu
};

struct mynfs_unlink_request
{
    uint16_t path_length; //długość ścieżki pliku
    char *path;           //ścieżka pliku
};

struct mynfs_unlink_response
{
    int16_t return_value; //0 w przypadku powodzenia lub kod błędu
};

struct mynfs_fstat_request
{
    int32_t file_descriptor; //deskryptor pliku
};

struct mynfs_fstat_response
{
    int16_t return_value;   //0 w przypadku powodzenia lub kod błędu
    struct stat file_stats; //struktura zawierająca informacje o pliku
};

//**********************************************************************************************************************

#endif //NFSLIB_HPP