//
// Created by 田地 on 2021/3/24.
//

#include <fcntl.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "../io.h"

int socket(int a, int b, int c) {
    return 9999;
}

ssize_t sendto(int fd, const void * buf, size_t size, int flag, const struct sockaddr * _sockaddr, socklen_t socklen) {
    return size;
}

ssize_t recvfrom(int fd, void * buf, size_t size, int flag, struct sockaddr * _sockaddr, socklen_t * socklen) {
    return size;
}

int inet_pton(int, const char *, void *) {
    return 1;
}

int bind(int, const struct sockaddr *, socklen_t) {
    return 0;
}

int CmpSockaddr(struct sockaddr* x, struct sockaddr* y) {
    return 0;
}