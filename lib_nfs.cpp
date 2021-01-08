#include "lib_nfs.h"

int mynfs_error;
std::map<char *, int> mynfs_open_count;
std::map<char *, int> mynfs_socket_descriptor;

int mynfs_open(char *host, char *path, int oflag, int mode)
{
    int mynfs_socket;
    auto open_file_check = mynfs_open_count.find(host);
    if ((open_file_check == mynfs_open_count.end()) || (open_file_check->second <= 0))
    {
        mynfs_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (mynfs_socket == -1)
        {
            mynfs_error = -1;
            return -1;
        }

        struct sockaddr_in mynfs_address;
        mynfs_address.sin_family = AF_INET;
        mynfs_address.sin_port = MYNFS_PORT_NUMBER;
        mynfs_address.sin_addr.s_addr = inet_addr(host);

        int result = connect(mynfs_socket, (struct sockaddr *)&mynfs_address, sizeof(mynfs_address));
        if (result == -1)
        {
            mynfs_error = -2;
            close(mynfs_socket);
            return -1;
        }

        mynfs_socket_descriptor[host] = mynfs_socket;
    }
    else
    {
        mynfs_socket = mynfs_socket_descriptor[host];
    }

    struct mynfs_open_request mynfs_request;
    mynfs_request.mode = mode;
    mynfs_request.oflag = oflag;
    mynfs_request.path_length = (strlen(path) + 1);

    uint8_t function_code = 1;
	uint8_t *function_type = &function_code;
    char *buffer = (char *)malloc(sizeof(mynfs_request.mode) + sizeof(mynfs_request.oflag) + sizeof(mynfs_request.path_length) + sizeof(char) * (strlen(path) + 1) + sizeof(uint8_t));
    memcpy(buffer, function_type, sizeof(uint8_t));
    memcpy(buffer + sizeof(uint8_t), &mynfs_request.path_length, sizeof(mynfs_request.path_length));
    memcpy(buffer + sizeof(uint8_t) + sizeof(mynfs_request.path_length), &mynfs_request.oflag, sizeof(mynfs_request.oflag));
    memcpy(buffer + sizeof(uint8_t) + sizeof(mynfs_request.path_length) + sizeof(mynfs_request.oflag), &mynfs_request.mode, sizeof(mynfs_request.mode));
    memcpy(buffer + sizeof(uint8_t) + sizeof(mynfs_request.path_length) + sizeof(mynfs_request.oflag) + sizeof(mynfs_request.mode), path, sizeof(char) * (strlen(path) + 1));

    ssize_t result = write(mynfs_socket, buffer, sizeof(uint8_t) + sizeof(mynfs_request.path_length) + sizeof(mynfs_request.oflag) + sizeof(mynfs_request.mode) + sizeof(char) * (strlen(path) + 1));
    free(buffer);
	if (result == -1)
    {
        mynfs_error = -3;
        if ((open_file_check == mynfs_open_count.end()) || (open_file_check->second <= 0))
        {
            close(mynfs_socket);
            mynfs_socket_descriptor[host] = -1;
        }
        return -1;
    }
    struct mynfs_open_response response;

    result = read(mynfs_socket, &response.return_value, sizeof(int32_t));
    if (result == 0)
    {
        close(mynfs_socket);
        mynfs_open_count[host] = 0;
        mynfs_error = -7;
    }
    if (result == -1)
    {
        mynfs_error = -4;
        if ((open_file_check == mynfs_open_count.end()) || (open_file_check->second <= 0))
        {
            close(mynfs_socket);
            mynfs_socket_descriptor[host] = -1;
        }

        return -1;
    }

    if (response.return_value <= 0)
    {
        if (response.return_value == 0)
        {
            mynfs_error = -5;
            close(mynfs_socket);
            mynfs_socket_descriptor[host] = -1;
            return -1;
        }
        else
        {
            mynfs_error = (-1) * response.return_value;
            if ((open_file_check == mynfs_open_count.end()) || (open_file_check->second <= 0))
            {
                close(mynfs_socket);
                mynfs_socket_descriptor[host] = -1;
            }

            return -1;
        }
    }

    if (open_file_check == mynfs_open_count.end())
    {
        mynfs_open_count[host] = 1;
    }
    else
    {
        mynfs_open_count[host] = mynfs_open_count[host] + 1;
    }

    return response.return_value;
}

int mynfs_read(char *host, int fds, void *buf, int count)
{
    int mynfs_socket;
    auto open_file_check = mynfs_open_count.find(host);
    if ((open_file_check == mynfs_open_count.end()) || (open_file_check->second <= 0))
    {
        mynfs_error = -6;
        return -1;
    }

    mynfs_socket = mynfs_socket_descriptor[host];

    struct mynfs_read_request mynfs_request;
    mynfs_request.file_descriptor = fds;
    mynfs_request.data_size = count;

    uint8_t function_code = 2;
    uint8_t *function_type = &function_code;
    char *buffer = (char *)malloc(sizeof(mynfs_request.file_descriptor) + sizeof(mynfs_request.data_size) + sizeof(uint8_t));
    memcpy(buffer, function_type, sizeof(uint8_t));
    memcpy(buffer + sizeof(uint8_t), &mynfs_request.file_descriptor, sizeof(mynfs_request.file_descriptor));
    memcpy(buffer + sizeof(uint8_t) + sizeof(mynfs_request.file_descriptor), &mynfs_request.data_size, sizeof(mynfs_request.data_size));

    ssize_t result = write(mynfs_socket, buffer, sizeof(uint8_t) + sizeof(mynfs_request.file_descriptor) + sizeof(mynfs_request.data_size));
    free(buffer);

    if (result == -1)
    {
        mynfs_error = -3;
        return -1;
    }
    struct mynfs_read_response response;

    result = read(mynfs_socket, &response.return_value, sizeof(int16_t));
    if (result == 0)
    {
        close(mynfs_socket);
        mynfs_open_count[host] = 0;
        mynfs_error = -7;
    }
    if (result == -1)
    {
        mynfs_error = -4;
        return -1;
    }

    if (response.return_value < 0)
    {
        mynfs_error = (-1) * response.return_value;
        return -1;
    }

    result = read(mynfs_socket, &response.data_size, sizeof(int16_t));
    if (result == 0)
    {
        close(mynfs_socket);
        mynfs_open_count[host] = 0;
        mynfs_error = -7;
    }
    if (result == -1)
    {
        mynfs_error = -4;
        return -1;
    }

    result = read(mynfs_socket, buf, sizeof(char) * response.data_size);
    if (result == 0)
    {
        close(mynfs_socket);
        mynfs_open_count[host] = 0;
        mynfs_error = -7;
    }
    if (result == -1)
    {
        mynfs_error = -4;
        return -1;
    }

    return response.data_size;
}

int mynfs_write(char *host, int fds, void *buf, int count)
{
    int mynfs_socket;
    auto open_file_check = mynfs_open_count.find(host);
    if ((open_file_check == mynfs_open_count.end()) || (open_file_check->second <= 0))
    {
        mynfs_error = -6;
        return -1;
    }

    mynfs_socket = mynfs_socket_descriptor[host];

    struct mynfs_write_request mynfs_request;
    mynfs_request.file_descriptor = fds;
    mynfs_request.data_size = count;

    uint8_t function_code = 3;
    uint8_t *function_type = &function_code;
    char *buffer = (char *)malloc(sizeof(mynfs_request.file_descriptor) + sizeof(mynfs_request.data_size) + sizeof(char) * mynfs_request.data_size + sizeof(uint8_t));
    memcpy(buffer, function_type, sizeof(uint8_t));
    memcpy(buffer + sizeof(uint8_t), &mynfs_request.file_descriptor, sizeof(mynfs_request.file_descriptor));
    memcpy(buffer + sizeof(uint8_t) + sizeof(mynfs_request.file_descriptor), &mynfs_request.data_size, sizeof(mynfs_request.data_size));
    memcpy(buffer + sizeof(uint8_t) + sizeof(mynfs_request.file_descriptor) + sizeof(mynfs_request.data_size), buf, sizeof(char) * mynfs_request.data_size);

    ssize_t result = write(mynfs_socket, buffer, sizeof(uint8_t) + sizeof(mynfs_request.file_descriptor) + sizeof(mynfs_request.data_size) + sizeof(char) * mynfs_request.data_size);
    free(buffer);
    if (result == -1)
    {
        mynfs_error = -3;
        return -1;
    }

    struct mynfs_write_response response;
    result = read(mynfs_socket, &response.return_value, sizeof(int16_t));
    if (result == 0)
    {
        close(mynfs_socket);
        mynfs_open_count[host] = 0;
        mynfs_error = -7;
    }
    if (result == -1)
    {
        mynfs_error = -4;
        return -1;
    }

    if (response.return_value < 0)
    {
        mynfs_error = (-1) * response.return_value;
        return -1;
    }

    return response.return_value;
}

int mynfs_lseek(char *host, int fds, int offset, int whence)
{
    int mynfs_socket;
    auto open_file_check = mynfs_open_count.find(host);
    if ((open_file_check == mynfs_open_count.end()) || (open_file_check->second <= 0))
    {
        mynfs_error = -6;
        return -1;
    }

    mynfs_socket = mynfs_socket_descriptor[host];

    struct mynfs_lseek_request mynfs_request;
    mynfs_request.file_descriptor = fds;
    mynfs_request.offset = offset;
    mynfs_request.whence = whence;

    uint8_t function_code = 4;
    uint8_t *function_type = &function_code;
    char *buffer = (char *)malloc(sizeof(mynfs_request.file_descriptor) + sizeof(mynfs_request.offset) + sizeof(mynfs_request.whence) + sizeof(uint8_t));
    memcpy(buffer, function_type, sizeof(uint8_t));
    memcpy(buffer + sizeof(uint8_t), &mynfs_request.file_descriptor, sizeof(mynfs_request.file_descriptor));
    memcpy(buffer + sizeof(uint8_t) + sizeof(mynfs_request.file_descriptor), &mynfs_request.offset, sizeof(mynfs_request.offset));
    memcpy(buffer + sizeof(uint8_t) + sizeof(mynfs_request.file_descriptor) + sizeof(mynfs_request.offset), &mynfs_request.whence, sizeof(mynfs_request.whence));

    ssize_t result = write(mynfs_socket, buffer, sizeof(uint8_t) + sizeof(mynfs_request.file_descriptor) + sizeof(mynfs_request.offset) + sizeof(mynfs_request.whence));
    free(buffer);
    if (result == -1)
    {
        mynfs_error = -3;
        return -1;
    }

    struct mynfs_write_response response;
    result = read(mynfs_socket, &response.return_value, sizeof(int16_t));
    if (result == 0)
    {
        close(mynfs_socket);
        mynfs_open_count[host] = 0;
        mynfs_error = -7;
    }
    if (result == -1)
    {
        mynfs_error = -4;
        return -1;
    }

    if (response.return_value < 0)
    {
        mynfs_error = (-1) * response.return_value;
        return -1;
    }

    return response.return_value;
}

int mynfs_close(char *host, int fds)
{

    int mynfs_socket;
    auto open_file_check = mynfs_open_count.find(host);
    if ((open_file_check == mynfs_open_count.end()) || (open_file_check->second <= 0))
    {
        mynfs_error = -6;
        return -1;
    }

    mynfs_socket = mynfs_socket_descriptor[host];

    struct mynfs_close_request mynfs_request;
    mynfs_request.file_descriptor = fds;

    uint8_t function_code = 5;
    uint8_t *function_type = &function_code;
    char *buffer = (char *)malloc(sizeof(uint8_t) + sizeof(mynfs_request.file_descriptor));
    memcpy(buffer, function_type, sizeof(uint8_t));
    memcpy(buffer + sizeof(uint8_t), &mynfs_request.file_descriptor, sizeof(mynfs_request.file_descriptor));

    ssize_t result = write(mynfs_socket, buffer, sizeof(uint8_t) + sizeof(mynfs_request.file_descriptor));
    free(buffer);
    if (result == -1)
    {
        mynfs_error = -3;
        return -1;
    }

    struct mynfs_close_response response;
    result = read(mynfs_socket, &response.return_value, sizeof(int16_t));
    if (result == 0)
    {
        close(mynfs_socket);
        mynfs_open_count[host] = 0;
        mynfs_error = -7;
    }
    if (result == -1)
    {
        mynfs_error = -4;
        mynfs_open_count[host] = mynfs_open_count[host] - 1;
        if (mynfs_open_count[host] <= 0)
        {
            mynfs_socket_descriptor[host] = -1;
            close(mynfs_socket);
        }
        return -1;
    }

    if (response.return_value < 0)
    {
        mynfs_error = (-1) * response.return_value;
        return -1;
    }

    mynfs_open_count[host] = mynfs_open_count[host] - 1;
    if (mynfs_open_count[host] <= 0)
    {
        mynfs_socket_descriptor[host] = -1;
        close(mynfs_socket);
    }

    return 0;
}

int mynfs_unlink(char *host, char *path)
{
    int mynfs_socket;
    bool opened_socket = false;
    auto open_file_check = mynfs_open_count.find(host);
    if ((open_file_check == mynfs_open_count.end()) || (open_file_check->second <= 0))
    {
        mynfs_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (mynfs_socket == -1)
        {
            mynfs_error = -1;
            return -1;
        }

        opened_socket = true;

        struct sockaddr_in mynfs_address;
        mynfs_address.sin_family = AF_INET;
        mynfs_address.sin_port = MYNFS_PORT_NUMBER;
        mynfs_address.sin_addr.s_addr = inet_addr(host);

        int result = connect(mynfs_socket, (struct sockaddr *)&mynfs_address, sizeof(mynfs_address));
        if (result == -1)
        {
            mynfs_error = -2;
            close(mynfs_socket);
            return -1;
        }
    }
    else
    {
        mynfs_socket = mynfs_socket_descriptor[host];
    }

    struct mynfs_unlink_request mynfs_request;

    mynfs_request.path_length = (strlen(path) + 1);
    uint8_t function_code = 6;
    uint8_t *function_type = &function_code;
    char *buffer = (char *)malloc(sizeof(mynfs_request.path_length) + sizeof(char) * (strlen(path) + 1) + sizeof(uint8_t));
    memcpy(buffer, function_type, sizeof(uint8_t));
    memcpy(buffer + sizeof(uint8_t), &mynfs_request.path_length, sizeof(mynfs_request.path_length));
    memcpy(buffer + sizeof(uint8_t) + sizeof(mynfs_request.path_length), path, sizeof(char) * (strlen(path) + 1));

    ssize_t result = write(mynfs_socket, buffer, sizeof(uint8_t) + sizeof(mynfs_request.path_length) + sizeof(char) * (strlen(path) + 1));
    free(buffer);
    if (result == -1)
    {
        mynfs_error = -3;
        if ((open_file_check == mynfs_open_count.end()) || (open_file_check->second <= 0))
        {
            close(mynfs_socket);
        }
        return -1;
    }

    struct mynfs_unlink_response response;
    result = read(mynfs_socket, &response.return_value, sizeof(int16_t));
    if (result == 0)
    {
        close(mynfs_socket);
        mynfs_open_count[host] = 0;
        mynfs_error = -7;
    }
    if (result == -1)
    {
        mynfs_error = -4;
        if ((open_file_check == mynfs_open_count.end()) || (open_file_check->second <= 0))
        {
            close(mynfs_socket);
        }
        return -1;
    }

    if (response.return_value < 0)
    {
        mynfs_error = (-1) * response.return_value;
        return -1;
    }

    if (opened_socket == true)
    {
        close(mynfs_socket);
    }

    return 0;
}

int mynfs_fstat(char *host, int fds, struct stat *buf)
{
    int mynfs_socket;
    auto open_file_check = mynfs_open_count.find(host);
    if ((open_file_check == mynfs_open_count.end()) || (open_file_check->second <= 0))
    {
        mynfs_error = -6;
        return -1;
    }

    mynfs_socket = mynfs_socket_descriptor[host];

    struct mynfs_fstat_request mynfs_request;
    mynfs_request.file_descriptor = fds;

    uint8_t function_code = 7;
    uint8_t *function_type = &function_code;
    char *buffer = (char *)malloc(sizeof(mynfs_request.file_descriptor) + sizeof(uint8_t));
    memcpy(buffer, function_type, sizeof(uint8_t));
    memcpy(buffer + sizeof(uint8_t), &mynfs_request.file_descriptor, sizeof(mynfs_request.file_descriptor));

    ssize_t result = write(mynfs_socket, buffer, sizeof(uint8_t) + sizeof(mynfs_request.file_descriptor));
    free(buffer);
    if (result == -1)
    {
        mynfs_error = -3;
        return -1;
    }
    struct mynfs_read_response response;
    result = read(mynfs_socket, &response.return_value, sizeof(int16_t));
    if (result == 0)
    {
        close(mynfs_socket);
        mynfs_open_count[host] = 0;
        mynfs_error = -7;
    }
    if (result == -1)
    {
        mynfs_error = -4;
        return -1;
    }

    if (response.return_value < 0)
    {
        mynfs_error = (-1) * response.return_value;
        return -1;
    }

    result = read(mynfs_socket, buf, sizeof(struct stat));
    if (result == 0)
    {
        close(mynfs_socket);
        mynfs_open_count[host] = 0;
        mynfs_error = -7;
    }
    if (result == -1)
    {
        mynfs_error = -4;
        return -1;
    }

    return 0;
}