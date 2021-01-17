#include "lib_nfs.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <utility>

class DescriptorStorage
{
public:
    int table[10][2];
    DescriptorStorage();
    void remove_element(int client_value);
    int element_count();
    int insert_element(int client_value, int server_value);
    int find_element(int client_descriptor);
    void close_all_elements();
    int next_free_descriptor();
};

DescriptorStorage::DescriptorStorage()
{
    int n;
    n = 0;
    while (n < 10)
    {
        table[n][0] = -1;
        table[n][1] = -1;
        n++;
    }
};

void DescriptorStorage::remove_element(int client_value)
{
    int n;
    n = 0;
    while (table[n][0] != client_value)
    {
        n++;
    }

    while (n < 9)
    {
        table[n][0] = table[n + 1][0];
        table[n][1] = table[n + 1][1];
        n++;
    }
    table[9][0] = -1;
    table[9][1] = -1;
};

int DescriptorStorage::element_count()
{
    int n, elem_count;
    n = 0;
    elem_count = 0;
    while (n < 10)
    {
        if (table[n][0] > 0 && table[n][1] > 0)
            elem_count++;
        n++;
    }
    return elem_count;
};

int DescriptorStorage::insert_element(int client_value, int server_value)
{
    int n;
    n = 0;
    if (this->element_count() == 10)
        return -1;
    while ( (table[n][0] > 0) && (table[n][1] > 0) && (n<10))
        {
        n++;
        }
    if (n >= 10)
        return -1;
    table[n][0] = client_value;
    table[n][1] = server_value;
    return n;
};

int DescriptorStorage::find_element(int client_descriptor)
{
    int n;
    n = 0;
    while (n < 10)
    {
        if (table[n][0] == client_descriptor)
            break;
        n++;
    }

    if (n == 10)
        return -1;
    else
        return table[n][1];
};

void DescriptorStorage::close_all_elements()
{
    int n;
    n = 0;
    while (n < 10)
    {
        if (table[n][0] > 0 && table[n][1] > 0)
        {
            flock(table[n][1], LOCK_UN);
            close(table[n][1]);
        }
        n++;
    }
};

int DescriptorStorage::next_free_descriptor()
{
    bool candidates[10];
    int n;
    n = 0;
    while (n < 10)
    {
        candidates[n] = true;
        n++;
    }
    n = 0;
    while (n < 10)
    {
        if (table[n][0] > 0)
            candidates[table[n][0] - 1] = false;
        n++;
    }
    n = 0;
    while (n < 10 && candidates[n] == false)
    {
        n++;
    }

    if (n == 10)
        return -1;
    else
        return n + 1;
};

void new_process(int fds);
bool handle_open(int socket, DescriptorStorage *descriptors);
bool handle_read(int socket, DescriptorStorage *descriptors);
bool handle_write(int socket, DescriptorStorage *descriptors);
bool handle_lseek(int socket, DescriptorStorage *descriptors);
bool handle_close(int socket, DescriptorStorage *descriptors);
bool handle_unlink(int socket, DescriptorStorage *descriptors);
bool handle_fstat(int socket, DescriptorStorage *descriptors);
void send_error_response(int error_code, int socket);
void close_all(DescriptorStorage *descriptor_list);

int main()
{
    int my_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (my_socket == -1)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }
    struct sockaddr_in my_address;
    my_address.sin_family = AF_INET;
    my_address.sin_port = MYNFS_PORT_NUMBER;
    my_address.sin_addr.s_addr = INADDR_ANY;
    ssize_t result = bind(my_socket, (struct sockaddr *)&my_address, sizeof(my_address));
    if (result == -1)
    {
        std::cerr << "Socket bind failed";
        return -1;
    }
    result = listen(my_socket, 50);
    if (result == -1)
    {
        std::cerr << "Socket listen failed";
        return -1;
    }
    int socket_descriptor;
    struct sockaddr new_address;
    socklen_t sock_len = sizeof(new_address);

    std::vector<std::string> whitelist;
    std::ifstream in_file("ip_addresses.txt");
    std::string adr;
    while (in_file >> adr)
        whitelist.push_back(adr);

    std::cout << "Listening" << std::endl;

    while (true)
    {
        socket_descriptor = accept(my_socket, &new_address, &sock_len);
        if (socket_descriptor == -1)
        {
            std::cerr << "Accept connection failed";
        }
        struct sockaddr_in *addrr = (struct sockaddr_in *)&new_address;
        (std::string) inet_ntoa(addrr->sin_addr);
        if (std::find(whitelist.begin(), whitelist.end(), adr) == whitelist.end())
        {
            send_error_response(0, socket_descriptor);
            close(socket_descriptor);
            continue;
        }

        pid_t pid = fork();
        if (pid == -1)
        {
            std::cerr << "Process creation failed";
        }
        else if (pid == 0)
            new_process(socket_descriptor);
    }
}

void new_process(int socket)
{
    //std::map<int, int> descriptors;
    //std::unordered_map<int, int> lol;
    DescriptorStorage descriptors;
    uint8_t function_id;
    bool loop = false;

    while (!loop)
    {
        ssize_t result = read(socket, &function_id, sizeof(uint8_t));
        if (result == 0)
        {
            close_all(&descriptors);
            break;
        }
        else if (result == -1)
        {
            std::cerr << "Subprocess read error " << errno;
            break;
        }
        switch ((int)function_id)
        {
        case 1:
            loop = handle_open(socket, &descriptors);
            break;

        case 2:
            loop = handle_read(socket, &descriptors);
            break;

        case 3:
            loop = handle_write(socket, &descriptors);
            break;

        case 4:
            loop = handle_lseek(socket, &descriptors);
            break;

        case 5:
            loop = handle_close(socket, &descriptors);
            break;

        case 6:
            loop = handle_unlink(socket, &descriptors);
            break;

        case 7:
            loop = handle_fstat(socket, &descriptors);
            break;

        default:
            std::cerr << "Unknown error";
            loop = true;
            break;
        }
    }

    close(socket);

    exit(0);
}

bool handle_open(int socket, DescriptorStorage *descriptors)
{
    if (descriptors->element_count() > 10)
        {
        char *buffer = (char *)malloc(sizeof(int16_t));
        ssize_t result = read(socket, buffer, sizeof(int16_t));
         if (result == 0)
            {
            close_all(descriptors);
            return true;
            }
        else if (result == -1)
            {
            std::cerr << "Internal error " << errno;
            close_all(descriptors);
            return true;
            }
        struct mynfs_open_request request;
        memcpy(&request.path_length, buffer, sizeof(int16_t));
        result = read(socket, buffer, sizeof(int16_t));
        result = read(socket, buffer, sizeof(int16_t));
		free(buffer);;
		buffer = (char *)malloc(request.path_length);
		result = read(socket, buffer, request.path_length);
		free(buffer);
		send_error_response(0, socket);
		return false;
		}

    char *buffer = (char *)malloc(sizeof(int16_t));
    ssize_t result = read(socket, buffer, sizeof(int16_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    struct mynfs_open_request request;
    memcpy(&request.path_length, buffer, sizeof(int16_t));
    result = read(socket, buffer, sizeof(int16_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    memcpy(&request.oflag, buffer, sizeof(int16_t));
    result = read(socket, buffer, sizeof(int16_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    memcpy(&request.mode, buffer, sizeof(int16_t));
    free(buffer);
    buffer = (char *)malloc(request.path_length);
    result = read(socket, buffer, request.path_length);

    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    request.path = (char *)malloc(request.path_length);
    memcpy(request.path, buffer, request.path_length);
    free(buffer);
    int file_descr = open(request.path, request.oflag, request.mode);
    if (file_descr == -1)
    {
        send_error_response(errno, socket);
        if (descriptors->element_count() == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    if (((request.mode & O_WRONLY) != 0) || ((request.mode & O_RDWR) != 0))
        result = flock(file_descr, LOCK_EX);
    else if ((request.mode & O_RDONLY) != 0)
        result = flock(file_descr, LOCK_SH);
    else
    {
        close(file_descr);
        std::cerr << "Unsupported open mode";
        if (descriptors->element_count() == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }


    if (result == -1)
    {
        send_error_response(errno, socket);
        if (descriptors->element_count() == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    int32_t ret_val;
    if (descriptors->element_count() < 10)
    {
        ret_val = descriptors->next_free_descriptor();
		descriptors->insert_element(ret_val, file_descr);
    }
    else
    {
        send_error_response(0, socket);
        close(file_descr);
        return false;
    }
    struct mynfs_open_response response;
    response.return_value = ret_val;

    buffer = (char *)malloc(sizeof(int32_t));
    memcpy(buffer, &response.return_value, sizeof(int32_t));

    result = write(socket, buffer, sizeof(int32_t));

    free(buffer);
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    return false;
}

bool handle_read(int socket, DescriptorStorage *descriptors)
{
    char *buffer = (char *)malloc(sizeof(int32_t));
    ssize_t result = read(socket, buffer, sizeof(int32_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    struct mynfs_read_request request;
    memcpy(&request.file_descriptor, buffer, sizeof(int32_t));
    free(buffer);
    buffer = (char *)malloc(sizeof(int16_t));
    result = read(socket, buffer, sizeof(int16_t));

    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    memcpy(&request.data_size, buffer, sizeof(int16_t));
    free(buffer);
    struct mynfs_read_response response;
    void *buf = malloc(request.data_size);

    int file_descr;
	file_descr = descriptors->find_element(request.file_descriptor);
	if (file_descr == -1)
	{
        send_error_response(errno, socket);
        return false;
    }
    else
    {
        result = read(file_descr, buf, request.data_size);
    }

    if (result == -1)
    {
        send_error_response(errno, socket);
        return false;
    }
    response.data_size = result;
    response.return_value = result;
    buffer = (char *)malloc(sizeof(int16_t));
    memcpy(buffer, &response.return_value, sizeof(int16_t));
    result = write(socket, buffer, sizeof(int16_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    memcpy(buffer, &response.data_size, sizeof(int16_t));
    result = write(socket, buffer, sizeof(int16_t));
    free(buffer);
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    result = write(socket, buf, response.data_size);
    free(buf);
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    return false;
}

bool handle_write(int socket, DescriptorStorage *descriptors)
{
    char *buffer;
    struct mynfs_write_request request;
    ssize_t result = read(socket, &request.file_descriptor, sizeof(int32_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    result = read(socket, &request.data_size, sizeof(int16_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    request.data = (char *)malloc(request.data_size);
    result = read(socket, request.data, request.data_size);
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }

    struct mynfs_write_response response;

    int file_descr;
	file_descr = descriptors->find_element(request.file_descriptor);
    if (file_descr == -1)
    {
        send_error_response(EBADF, socket);
        return false;
    }
    else
    {
        result = write(file_descr, request.data, request.data_size);
    }


    if (result == -1)
    {

        send_error_response(errno, socket);
        return false;
    }
    response.return_value = result;
    buffer = (char *)malloc(sizeof(int16_t));
    memcpy(buffer, &response.return_value, sizeof(int16_t));
    result = write(socket, buffer, sizeof(int16_t));
    free(buffer);
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    return false;
}

bool handle_lseek(int socket, DescriptorStorage *descriptors)
{
    char *buffer = (char *)malloc(sizeof(int32_t));
    ssize_t result = read(socket, buffer, sizeof(int32_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    struct mynfs_lseek_request request;
    memcpy(&request.file_descriptor, buffer, sizeof(int32_t));
    free(buffer);
    buffer = (char *)malloc(sizeof(int16_t));
    result = read(socket, buffer, sizeof(int16_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    memcpy(&request.offset, buffer, sizeof(int16_t));
    result = read(socket, buffer, sizeof(int16_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    memcpy(&request.whence, buffer, sizeof(int16_t));
    free(buffer);
    struct mynfs_lseek_response response;
    
    int file_descr;
	file_descr = descriptors->find_element(request.file_descriptor);
    if (file_descr == -1)
    {
        send_error_response(EBADF, socket);
        return false;
    }
    else
    {
        result = lseek(file_descr, request.offset, request.whence);
    }

    if (result == -1)
    {

        send_error_response(errno, socket);
        return false;
    }
    response.return_value = result;
    buffer = (char *)malloc(sizeof(int16_t));
    memcpy(buffer, &response.return_value, sizeof(int16_t));
    result = write(socket, buffer, sizeof(int16_t));
    free(buffer);
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    return false;
}

bool handle_close(int socket, DescriptorStorage *descriptors)
{
    char *buffer = (char *)malloc(sizeof(int32_t));
    ssize_t result = read(socket, buffer, sizeof(int32_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    struct mynfs_close_request request;
    memcpy(&request.file_descriptor, buffer, sizeof(int32_t));
    free(buffer);
    struct mynfs_close_response response;

    int file_descr;
	file_descr = descriptors->find_element(request.file_descriptor);
    if (file_descr == -1)
    {
        send_error_response(EBADF, socket);
        return false;
    }
    else
    {
        result = close(file_descr);
        descriptors->remove_element(request.file_descriptor);
	}

	if (result == -1)
    {
        send_error_response(errno, socket);
        return false;
    }
    response.return_value = result;
    buffer = (char *)malloc(sizeof(int16_t));
    memcpy(buffer, &response.return_value, sizeof(int16_t));
    result = write(socket, buffer, sizeof(int16_t));
    free(buffer);
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    return false;
}

bool handle_unlink(int socket, DescriptorStorage *descriptors)
{
    char *buffer = (char *)malloc(sizeof(int16_t));
    ssize_t result = read(socket, buffer, sizeof(int16_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    struct mynfs_unlink_request request;
    memcpy(&request.path_length, buffer, sizeof(int16_t));
    free(buffer);
    buffer = (char *)malloc(request.path_length);
    result = read(socket, buffer, request.path_length);
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    request.path = (char *)malloc(request.path_length);
    memcpy(request.path, buffer, request.path_length);
    free(buffer);
    result = unlink(request.path);
    if (result == -1)
    {
        send_error_response(errno, socket);
        if (descriptors->element_count() == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    struct mynfs_unlink_response response;
    response.return_value = result;
    buffer = (char *)malloc(sizeof(int16_t));
    memcpy(buffer, &response.return_value, sizeof(int16_t));
    result = write(socket, buffer, sizeof(int16_t));
    free(buffer);
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    return false;
}

bool handle_fstat(int socket, DescriptorStorage *descriptors)
{
    char *buffer = (char *)malloc(sizeof(int32_t));
    ssize_t result = read(socket, buffer, sizeof(int32_t));
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    struct mynfs_fstat_request request;
    memcpy(&request.file_descriptor, buffer, sizeof(int32_t));
    free(buffer);
    struct mynfs_fstat_response response;
	struct stat f_status;

	int file_descr;
	file_descr = descriptors->find_element(request.file_descriptor);
    if (file_descr == -1)
    {
        send_error_response(EBADF, socket);
        return false;
    }
    else
    {
        result = fstat(file_descr, &f_status);
    }

    if (result == -1)
    {
        send_error_response(errno, socket);
        return false;
    }

    response.return_value = result;
    buffer = (char *)malloc(sizeof(int16_t));
    memcpy(buffer, &response.return_value, sizeof(int16_t));
    result = write(socket, buffer, sizeof(int16_t));
    free(buffer);
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    buffer = (char *)malloc(sizeof(f_status));
    memcpy(buffer, &f_status, sizeof(f_status)); 
    result = write(socket, buffer, sizeof(f_status));
    free(buffer);
    if (result == 0)
    {
        close_all(descriptors);
        return true;
    }
    else if (result == -1)
    {
        std::cerr << "Internal error " << errno;
        close_all(descriptors);
        return true;
    }
    return false;
}

void send_error_response(int error_code, int socket)
{
    struct mynfs_open_response response;
    response.return_value = (-1) * error_code;
    char *buffer = (char *)malloc(sizeof(int32_t));
    memcpy(buffer, &response.return_value, sizeof(int32_t));
    write(socket, buffer, sizeof(int32_t));
    free(buffer);
}

//******************************************************************************************************************
void close_all(DescriptorStorage *descriptor_list)
{
    descriptor_list->close_all_elements();
}