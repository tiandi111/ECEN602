//
// Created by 田地 on 2021/1/28.
//

#ifndef PROJ1_UTILS_H
#define PROJ1_UTILS_H

#include <sys/types.h>

#include <cstddef>
#include <cerrno>

namespace echo {
    /*
     * WriteSocket writes len characters from buf into the socket.
     * will retry until all len characters has been written.
     * @param sockfd    target socket.
     * @param buf       char buffer.
     * @param len       length to write.
     * @return upon successful write, the length of characters is returned; otherwise, return -1.
     */
    ssize_t WriteSocket(int sockfd, const void *buf, int len);

    /*
     * CloseSocket closes the given socket.
     * @param sockfd    socket to close.
     * @param upon successful close, return 0; otherwise, return -1.
     */
    int CloseSocket(int sockfd);
} // namespace echo

#endif //PROJ1_UTILS_H
