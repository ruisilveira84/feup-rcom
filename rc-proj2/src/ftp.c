#include "ftp.h"
#include "parse.h"

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>

#define SERVER_PORT 21
#define BUFFER_SIZE 1024

#define LOGIN_SUCCESS 230

#define POST_STATUS_CODE 4

#define FTP_WRITE(fd, buf, label, format, ...)      memset(buf, 0, BUFFER_SIZE);                                                \
                                                    sprintf(buf, format __VA_OPT__(,) __VA_ARGS__);                             \
                                                    if (write(fd, buf, strlen(buf)) < 0) {                                      \
                                                        printf("[ERROR] Failed to write to socket: %s.\n", strerror(errno));    \
                                                        return_code = -1; goto label;                                           \
                                                    }

#define FTP_RECV(fd, buf, label)                    memset(buf, 0, BUFFER_SIZE);                                                \
                                                    if (recv(fd, buf, BUFFER_SIZE, 0) < 0) {                                    \
                                                        printf("[ERROR] Failed to read from socket: %s\n.", strerror(errno));   \
                                                        return_code = -1; goto label;                                           \
                                                    }

char *get_ip(char *hostname) {
    struct hostent *host_info;

    host_info = gethostbyname(hostname);
    if (host_info == NULL) {
        printf("[ERROR] Failed to get host's IP address: %s.\n", hstrerror(h_errno));
        return NULL;
    }

    return inet_ntoa(*((struct in_addr *)host_info->h_addr));
}

int ftp(url_params *params, char *filename) {

    int return_code = 0;
    size_t filesize = 0;

    char *ip = get_ip(params->hostname);
    if (ip == NULL) return - 1;

    int ctrl_socket;

    struct sockaddr_in ctrlsockaddr_info;
    memset(&ctrlsockaddr_info, 0, sizeof(struct sockaddr_in));
    ctrlsockaddr_info.sin_family = AF_INET;
    ctrlsockaddr_info.sin_addr.s_addr = inet_addr(ip);
    ctrlsockaddr_info.sin_port = htons(SERVER_PORT);

    if ((ctrl_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[ERROR] Failed to open socket: %s.\n", strerror(errno));
        return_code = -1; goto exit;
    }

    if (connect(ctrl_socket, (struct sockaddr*)&ctrlsockaddr_info, sizeof(ctrlsockaddr_info)) < 0) {
        printf("[ERROR] Failed to connect to server: %s.\n", strerror(errno));
        return_code = -1; goto close_ctrl;
    }

    char buffer[BUFFER_SIZE] = {0};

    FTP_RECV(ctrl_socket, buffer, close_ctrl);

    FTP_WRITE(ctrl_socket, buffer, close_ctrl, "USER %s\r\n", params->username);
    FTP_RECV(ctrl_socket, buffer, close_ctrl);

    FTP_WRITE(ctrl_socket, buffer, close_ctrl, "PASS %s\r\n", params->password);
    FTP_RECV(ctrl_socket, buffer, close_ctrl);

    if (strtol(buffer, NULL, 10) != LOGIN_SUCCESS) {
        printf("[ERROR] Invalid credentials.\n");
        return_code = -1; goto close_ctrl;
    }

    FTP_WRITE(ctrl_socket, buffer, close_ctrl, "TYPE I\r\n");
    FTP_RECV(ctrl_socket, buffer, close_ctrl);

    FTP_WRITE(ctrl_socket, buffer, close_ctrl, "SIZE %s\r\n", filename);
    FTP_RECV(ctrl_socket, buffer, close_ctrl);

    filesize = strtoul(&buffer[POST_STATUS_CODE], NULL, 10);

    FTP_WRITE(ctrl_socket, buffer, close_ctrl, "PASV\r\n");
    FTP_RECV(ctrl_socket, buffer, close_ctrl);

    long port;
    if ((port = parse_port(buffer)) < 0) {
        return_code = 1; goto close_ctrl;
    }

    int recv_socket;

    struct sockaddr_in recvsockaddr_info;
    memset(&recvsockaddr_info, 0, sizeof(struct sockaddr_in));
    recvsockaddr_info.sin_family = AF_INET;
    recvsockaddr_info.sin_addr.s_addr = inet_addr(ip);
    recvsockaddr_info.sin_port = htons(port);

    if ((recv_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[ERROR] Failed to open socket: %s.\n", strerror(errno));
        return_code = -1; goto close_ctrl;
    }

    if (connect(recv_socket, (struct sockaddr*)&recvsockaddr_info, sizeof(ctrlsockaddr_info)) < 0) {
        printf("[ERROR] Failed to connect to server: %s.\n", strerror(errno));
        return_code = -1; goto close_recv;
    }

    FTP_WRITE(ctrl_socket, buffer, close_recv, "RETR %s\r\n", params->path);

    int fd;

    if ((fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0666)) < 0) {
        printf("[ERROR] Failed to create file: %s.\n", strerror(errno));
        return_code = -1; goto close_recv;
    }

    int bytes;
    unsigned char *file_buffer = malloc(filesize);

    while ((bytes = recv(recv_socket, file_buffer, filesize, 0)) != 0) {
        if (bytes < 0) {
            printf("[ERROR] Failed to receive file: %s.\n", strerror(errno));
            return_code = -1; break;
        }

        if (write(fd, file_buffer, bytes) < 0) {
            printf("[ERROR] Failed to write to file: %s.\n", strerror(errno));
            return_code = -1; break;
        }
    }

    free(file_buffer);

    if (close(fd) < 0) {
        printf("[ERROR] Failed to close file: %s.\n", strerror(errno));
        return_code = -1;
    }

close_recv:

    if (close(recv_socket) < 0) {
        printf("[ERROR] Failed to close client socket: %s.\n", strerror(errno));
        return_code = -1;
    }

close_ctrl:

    if (close(ctrl_socket) < 0) {
        printf("[ERROR] Failed to close server control socket: %s.\n", strerror(errno));
        return_code = -1;
    }

exit:

    return return_code;
}