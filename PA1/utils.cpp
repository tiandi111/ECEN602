//
// Created by 田地 on 2021/1/28.
//

#include "utils.h"

#include <sys/socket.h>

#include <cerrno>
#include <cstring>

ssize_t echo::WriteSocket(int sockfd, const void * buf, int len) {
    ssize_t sent;
    ssize_t total = 0;

    do {

        for (sent = send(sockfd, buf, len, 0); sent >= 0 && total < len; total += sent) {
        }

    } while (sent == -1 && errno == EINTR);

    return total;
}
